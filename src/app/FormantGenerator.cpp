#include "FormantGenerator.h"

using nativeformat::param::createParam;

FormantGenerator::FormantGenerator(const AudioTime&           time,
                                   const std::vector<double>& input)
    : BufferedGenerator(time),
      m_filters({{800, 0, 80},
                 {1150, -6, 90},
                 {2900, -32, 120},
                 {3900, -20, 130},
                 {4650, -50, 140}}),
      m_input(input) {
    for (int k = 0; k < kNumFormants; ++k) {
        m_targetF[k] = m_filters[k].frequency();
        m_targetB[k] = m_filters[k].bandwidth();
        m_targetG[k] = m_filters[k].gain();
    }
}

double FormantGenerator::frequency(const int k) const { return m_targetF[k]; }

void FormantGenerator::setFrequency(const int k, const double Fk) {
    if (m_F[k]) {
        m_F[k]->exponentialRampToValueAtTime(Fk, time() + 0.1);
    } else {
        m_F[k] = createParam(Fk, 6000.0, 200.0, "F" + std::to_string(k + 1));
    }
    m_targetF[k] = Fk;
}

double FormantGenerator::bandwidth(const int k) const { return m_targetB[k]; }

void FormantGenerator::setBandwidth(const int k, const double Bk) {
    if (m_B[k]) {
        m_B[k]->exponentialRampToValueAtTime(Bk, time() + 0.1);
    } else {
        m_B[k] = createParam(Bk, 600.0, 10.0, "B" + std::to_string(k + 1));
    }
    m_targetB[k] = Bk;
}

double FormantGenerator::gain(const int k) const { return m_targetG[k]; }

void FormantGenerator::setGain(const int k, const double Gk) {
    if (m_G[k]) {
        m_G[k]->linearRampToValueAtTime(Gk, time() + 0.1);
    } else {
        m_G[k] = createParam(Gk, 5.0, -200.0, "G" + std::to_string(k + 1));
    }
    m_targetG[k] = Gk;
}

void FormantGenerator::setSampleRate(const double fs) {
    m_fs = fs;
    for (auto& filter : m_filters) {
        filter.setSampleRate(fs);
    }
}

void FormantGenerator::fillInternalBuffer(std::vector<double>& out) {
    for (int i = 0; i < out.size(); ++i) {
        const double t = time(i);

        double y = 0;

        for (int k = 0; k < kNumFormants; ++k) {
            if (m_F[k]) m_filters[k].setFrequency(m_F[k]->valueForTime(t));
            if (m_B[k]) m_filters[k].setBandwidth(m_B[k]->valueForTime(t));
            if (m_G[k]) m_filters[k].setGain(m_G[k]->valueForTime(t));

            // If input[i] > 0, we're roughly in the open phase, shift formant frequency.
            if (m_input[i] > 0) {
                m_filters[k].setFrequencyMultiplier(1 + 0.01 * m_input[i]);
                m_filters[k].setQualityMultiplier(1 - 0.1 * m_input[i]);
                m_filters[k].setGainOffset(-6 * m_input[i]);
            } else {
                m_filters[k].setFrequencyMultiplier(1);
                m_filters[k].setQualityMultiplier(1);
                m_filters[k].setGainOffset(1 * -m_input[i]);
            }

            m_filters[k].update();
            y += m_filters[k].tick(m_input[i]);
        }

        out[i] = y;
    }

    // Prune past parameter events
    for (int k = 0; k < kNumFormants; ++k) {
        if (m_F[k]) m_F[k]->pruneEventsPriorToTime(time());
        if (m_B[k]) m_B[k]->pruneEventsPriorToTime(time());
        if (m_G[k]) m_G[k]->pruneEventsPriorToTime(time());
    }
}