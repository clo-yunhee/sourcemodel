#include "FormantGenerator.h"

using nativeformat::param::createParam;

FormantGenerator::FormantGenerator(const AudioTime&           time,
                                   const std::vector<double>& input)
    : BufferedGenerator(time), m_mustRegenSpectrum(true), m_input(input) {
    setSampleRate(48000);
    // Initializing it manually instead of an initializer list constructor because
    // for some reason Emscripten doesn't like it
    double initial[][3] = {{800, 0, 80},
                           {1150, -6, 90},
                           {2900, -32, 120},
                           {3900, -20, 130},
                           {4650, -50, 140}};

    for (int k = 0; k < kNumFormants; ++k) {
        const double fk = initial[k][0];
        const double gk = initial[k][1];
        const double bk = initial[k][2];

        m_filters[k].setFrequency(fk);
        m_filters[k].setGain(gk);
        m_filters[k].setBandwidth(bk);

        m_targetF[k] = m_filters[k].frequency();
        m_targetB[k] = m_filters[k].bandwidth();
        m_targetG[k] = m_filters[k].gain();
    }

    m_lipRadiationMemory = 0;
}

double FormantGenerator::frequency(const int k) const { return m_targetF[k]; }

void FormantGenerator::setFrequency(const int k, const double Fk) {
    if (m_F[k]) {
        m_F[k]->linearRampToValueAtTime(Fk, time() + 0.15);
    } else {
        m_F[k] = createParam(Fk, 6000.0, 200.0, "F" + std::to_string(k + 1));
    }
    m_targetF[k] = Fk;
}

double FormantGenerator::bandwidth(const int k) const { return m_targetB[k]; }

void FormantGenerator::setBandwidth(const int k, const double Bk) {
    if (m_B[k]) {
        m_B[k]->linearRampToValueAtTime(Bk, time() + 0.15);
    } else {
        m_B[k] = createParam(Bk, 600.0, 10.0, "B" + std::to_string(k + 1));
    }
    m_targetB[k] = Bk;
}

double FormantGenerator::gain(const int k) const { return m_targetG[k]; }

void FormantGenerator::setGain(const int k, const double Gk) {
    if (m_G[k]) {
        m_G[k]->linearRampToValueAtTime(Gk, time() + 0.15);
    } else {
        m_G[k] = createParam(Gk, 5.0, -200.0, "G" + std::to_string(k + 1));
    }
    m_targetG[k] = Gk;
}

const FilterSpectrum& FormantGenerator::spectrum() const { return m_spectrum; }

FilterSpectrum& FormantGenerator::spectrum() { return m_spectrum; }

void FormantGenerator::updateSpectrumIfNeeded() {
    if (m_mustRegenSpectrum) {
        updateSpectrum();
        m_mustRegenSpectrum = false;
    }
}

void FormantGenerator::fillInternalBuffer(std::vector<double>& out) {
    if (hasSampleRateChanged()) {
        for (auto& filter : m_filters) {
            filter.setSampleRate(fs());
        }
        m_spectrum.setSampleRate(fs());
        ackSampleRateChange();
    }

    const double     d = m_lipRadiationCoeff;
    constexpr double g = 0.25;  // Lip filter normalized to -6dB gain at DC.

    for (int i = 0; i < out.size(); ++i) {
        const double t = time(i);

        double y = m_input[i];

        for (int k = 0; k < kNumFormants; ++k) {
            if (m_F[k]) m_filters[k].setFrequency(m_F[k]->valueForTime(t));
            if (m_B[k]) m_filters[k].setBandwidth(m_B[k]->valueForTime(t));
            if (m_G[k]) m_filters[k].setGain(m_G[k]->valueForTime(t));

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
        if (m_G[k]) m_G[k]->pruneEventsPriorToTime(time());
    }

    m_mustRegenSpectrum = true;
}

void FormantGenerator::updateSpectrum() {
    std::vector<std::array<double, 6>> sos(kNumFormants + 1);
    for (int i = 0; i < kNumFormants; ++i) {
        sos[i] = m_filters[i].getBiquadCoefficients();
    }
    const double d = m_lipRadiationCoeff;
    sos[kNumFormants] = {1 / (1 - d), -d / (1 - d), 0, 1, 0, 0};
    m_spectrum.update(sos);
}