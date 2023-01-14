#include "FormantGenerator.h"

using nativeformat::param::createParam;

FormantGenerator::FormantGenerator(const AudioTime&           time,
                                   const std::vector<Scalar>& input)
    : BufferedGenerator(time), m_mustRegenSpectrum(true), m_input(input) {
    // Initializing it manually instead of an initializer list constructor because
    // for some reason Emscripten doesn't like it
    Scalar initial[][2] = {{800, 80}, {1150, 90}, {2900, 120}, {3900, 130}, {4650, 140}};

    for (int k = 0; k < kNumFormants; ++k) {
        const Scalar fk = initial[k][0];
        const Scalar bk = initial[k][1];

        m_filters[k].setFrequency(fk);
        m_filters[k].setBandwidth(bk);

        m_targetF[k] = m_filters[k].frequency();
        m_targetB[k] = m_filters[k].bandwidth();
    }

    m_lipRadiationMemory = 0;
}

Scalar FormantGenerator::frequency(const int k) const { return m_targetF[k]; }

void FormantGenerator::setFrequency(const int k, const Scalar Fk) {
    if (m_F[k]) {
        m_F[k]->linearRampToValueAtTime(Fk, time() + 0.15_f);
    } else {
        m_F[k] = createParam(Fk, 6000.0_f, 200.0_f, "F" + std::to_string(k + 1));
    }
    m_targetF[k] = Fk;
}

Scalar FormantGenerator::bandwidth(const int k) const { return m_targetB[k]; }

void FormantGenerator::setBandwidth(const int k, const Scalar Bk) {
    if (m_B[k]) {
        m_B[k]->linearRampToValueAtTime(Bk, time() + 0.15_f);
    } else {
        m_B[k] = createParam(Bk, 600.0_f, 10.0_f, "B" + std::to_string(k + 1));
    }
    m_targetB[k] = Bk;
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

        for (int k = 0; k < kNumFormants; ++k) {
            if (m_F[k]) m_filters[k].setFrequency(m_F[k]->valueForTime(t));
            if (m_B[k]) m_filters[k].setBandwidth(m_B[k]->valueForTime(t));

            m_filters[k].update();
            y = m_filters[k].tick(y);
        }

        // Lip radiation filter is a 1st order FIR filter.
        out[i] = g / (1 - d) * y - g * d / (1 - d) * m_lipRadiationMemory;

        m_lipRadiationMemory = y;
    }

    // Prune past parameter events
    for (int k = 0; k < kNumFormants; ++k) {
        if (m_F[k]) m_F[k]->pruneEventsPriorToTime(time());
        if (m_B[k]) m_B[k]->pruneEventsPriorToTime(time());
    }

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