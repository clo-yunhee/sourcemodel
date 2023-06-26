#include "FormantGenerator.h"

#include <boost/math/special_functions/sin_pi.hpp>

using namespace std::placeholders;

using boost::math::sin_pi;
using nativeformat::param::createParam;

namespace {
constexpr Scalar minFv = 200;
constexpr Scalar maxFv = 6000;
constexpr Scalar minBv = 10;
constexpr Scalar maxBv = 600;

constexpr std::array<Scalar, 13> jitterDistributionWeights = {
    1 / 64., 2 / 64., 3 / 64., 5 / 64., 7 / 64., 9 / 64., 10 / 64.,
    9 / 64., 7 / 64., 5 / 64., 3 / 64., 2 / 64., 1 / 64.};

constexpr std::array<Scalar, 13> jitterDistributionDeviations = {
    -3, -1.9, -1.48, -1.12, -0.76, -0.38, 0, 0.38, 0.76, 1.12, 1.48, 1.9, 3};
}  // namespace

FormantGenerator::FormantGenerator(const AudioTime&           time,
                                   const std::vector<Scalar>& input)
    : BufferedGenerator(time),
      m_mustRegenSpectrum(true),
      m_input(input),
      m_targetF({{"F1", 800, minFv, maxFv},
                 {"F2", 1150, minFv, maxFv},
                 {"F3", 2900, minFv, maxFv},
                 {"F4", 3900, minFv, maxFv},
                 {"F5", 4650, minFv, maxFv}}),
      m_targetB({{"B1", 80, minBv, maxBv},
                 {"B2", 90, minBv, maxBv},
                 {"B3", 120, minBv, maxBv},
                 {"B4", 130, minBv, maxBv},
                 {"B5", 140, minBv, maxBv}}),
      m_paramFlutter("Ffmax", 0.03, 0, 0.5),
      m_paramFlutterToggle("Ffon", true) {
    m_paramFlutter.valueChanged.connect(&FormantGenerator::handleParamChanged, this);
    m_paramFlutterToggle.valueChanged.connect(&FormantGenerator::handleParamChanged,
                                              this);

    m_Ffmax = m_paramFlutter.createParamFrom();

    // Initializing it manually instead of an initializer list constructor because
    // for some reason Emscripten doesn't like it
    // Scalar initial[][2] = {{800, 80}, {1150, 90}, {2900, 120}, {3900, 130}, {4650,
    // 140}};

    for (int k = 0; k < kNumFormants; ++k) {
        const Scalar fk = m_targetF[k].value();
        const Scalar bk = m_targetB[k].value();

        m_filters[k].setFrequency(fk);
        m_filters[k].setBandwidth(bk);

        m_targetF[k].setValue(m_filters[k].frequency());
        m_targetB[k].setValue(m_filters[k].bandwidth());

        m_targetF[k].valueChanged.connect(std::bind(
            std::mem_fn(&FormantGenerator::handleFrequencyChanged), this, k, _1, _2));
        m_targetB[k].valueChanged.connect(std::bind(
            std::mem_fn(&FormantGenerator::handleBandwidthChanged), this, k, _1, _2));

        m_F[k] = createParam(m_targetF[k].value(), maxFv, minFv, m_targetF[k].name());
        m_B[k] = createParam(m_targetB[k].value(), maxBv, minBv, m_targetB[k].name());
    }

    m_lipRadiationMemory = 0;
}

ScalarParameter& FormantGenerator::frequency(const int k) { return m_targetF[k]; }

ScalarParameter& FormantGenerator::bandwidth(const int k) { return m_targetB[k]; }

ScalarParameter& FormantGenerator::flutter() { return m_paramFlutter; }

ToggleParameter& FormantGenerator::flutterToggle() { return m_paramFlutterToggle; }

void FormantGenerator::handleFrequencyChanged(const int k, const std::string& name,
                                              const Scalar Fk) {
    m_F[k]->linearRampToValueAtTime(Fk, time() + 0.15_f);
}

void FormantGenerator::handleBandwidthChanged(const int k, const std::string& name,
                                              const Scalar Bk) {
    m_B[k]->linearRampToValueAtTime(Bk, time() + 0.15_f);
}

void FormantGenerator::handleParamChanged(const std::string& name, const Scalar value) {
    if (name == "Ffmax") {
        m_Ffmax->linearRampToValueAtTime(value, time() + 0.1_f);
    } else if (name == "Ffon") {
        // If on => set Ffmax to current value of paramFlutter
        // If off => set Ffmax to 0
        m_Ffmax->linearRampToValueAtTime(value * m_paramFlutter.value(), time() + 0.1_f);
    }
}

const FilterSpectrum& FormantGenerator::spectrum() const { return m_spectrum; }

FilterSpectrum& FormantGenerator::spectrum() { return m_spectrum; }

void FormantGenerator::updateSpectrumIfNeeded() {
    if (m_mustRegenSpectrum) {
        updateSpectrum();
        m_mustRegenSpectrum = false;
    }
}

void FormantGenerator::fillInternalBuffer(std::vector<Scalar>& out) {
    if (hasSampleRateChanged()) {
        for (auto& filter : m_filters) {
            filter.setSampleRate(fs());
        }
        m_spectrum.setSampleRate(fs());
        ackSampleRateChange();
    }

    const Scalar     d = m_lipRadiationCoeff;
    constexpr Scalar g = 0.25_f;  // Lip filter normalized to -6dB gain at DC.

    for (int i = 0; i < out.size(); ++i) {
        const Scalar t = time(i);

        Scalar y = m_input[i];

        const Scalar Ffmax = m_Ffmax->valueForTime(t);

        for (int k = 0; k < kNumFormants; ++k) {
            const Scalar Fk = m_F[k]->valueForTime(t);
            const Scalar Bk = m_B[k]->valueForTime(t);

            // Mod each formant time
            const Scalar tk = (1 - .2 * (k - kNumFormants / 2)) * (t + .5 * k);
            const Scalar Fln = .1 * (sin_pi(2 * 12.7 * tk) + sin_pi(2 * 7.1 * tk) +
                                     sin_pi(2 * 4.7 * tk));

            m_filters[k].setFrequency(Fk * (1 + Ffmax * Fln));
            m_filters[k].setBandwidth(Bk * (1 + Ffmax * Fln));

            m_filters[k].update();
            y = m_filters[k].tick(y);
        }

        // Lip radiation filter is a 1st order FIR filter.
        out[i] = g / (1 - d) * y - g * d / (1 - d) * m_lipRadiationMemory;

        m_lipRadiationMemory = y;
    }

    // Prune past parameter events
    for (int k = 0; k < kNumFormants; ++k) {
        m_F[k]->pruneEventsPriorToTime(time());
        m_B[k]->pruneEventsPriorToTime(time());
    }
    m_Ffmax->pruneEventsPriorToTime(time());

    m_mustRegenSpectrum = true;
}

void FormantGenerator::updateSpectrum() {
    std::vector<std::array<Scalar, 6>> sos(kNumFormants + 1);
    for (int i = 0; i < kNumFormants; ++i) {
        sos[i] = m_filters[i].getBiquadCoefficients();
    }
    const Scalar d = m_lipRadiationCoeff;
    sos[kNumFormants] = {1 / (1 - d), -d / (1 - d), 0, 1, 0, 0};
    m_spectrum.update(sos);
}