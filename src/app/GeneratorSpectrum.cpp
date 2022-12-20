#include "GeneratorSpectrum.h"

#include <boost/math/constants/constants.hpp>

#include "audio/BufferedGenerator.h"
#include "math/utils.h"

using namespace boost::math::double_constants;

GeneratorSpectrum::GeneratorSpectrum(BufferedGenerator *initialGenerator)
    : m_generator(initialGenerator) {
    setTransformSize(8192);
    setSmoothing(0.1);
    setSampleRate(48000);
}

void GeneratorSpectrum::setTransformSize(const int nfft) {
    if (m_nfft != nfft) {
        m_nfft = nfft;
        m_dtft.setSampleCount(nfft);
        m_generator->setBufferLength(nfft);
        // Reconstruct window.
        m_window.resize(m_nfft);
        m_values.resize(m_nfft);
        for (int i = 0; i < m_nfft; ++i) {
            // Hann
            m_window[i] = 0.5 * (1 - cos((two_pi * i) / (m_nfft - 1)));
        }
        // Reconstruct frequency array.
        m_freqs.resize(m_dtft.binCount());
        m_mags.resize(m_dtft.binCount());
        m_spls.resize(m_dtft.binCount());
        for (int i = 0; i < m_dtft.binCount(); ++i) {
            m_freqs[i] = (i * m_fs) / m_nfft;
            m_mags[i] = 0;
            m_spls[i] = std::numeric_limits<double>::lowest();
        }
    }
}

void GeneratorSpectrum::setSmoothing(const double alpha) { m_alpha = alpha; }

void GeneratorSpectrum::setSampleRate(const double fs) {
    if (!fuzzyEquals(m_fs, fs)) {
        m_fs = fs;
        // Reconstruct frequency array.
        for (int i = 0; i < m_dtft.binCount(); ++i) {
            m_freqs[i] = (i * m_fs) / m_nfft;
            m_mags[i] = 0;
            m_spls[i] = std::numeric_limits<double>::lowest();
        }
    }
}

void GeneratorSpectrum::setGenerator(BufferedGenerator *generator) {
    m_generator = generator;
    generator->setBufferLength(m_nfft);
}

void GeneratorSpectrum::update() {
    m_generator->copyBufferTo(m_values);

    for (int i = 0; i < m_nfft; ++i) {
        m_values[i] *= m_window[i];
    }

    m_dtft.updateSamples(m_values.data(), m_nfft);

    for (int i = 0; i < m_dtft.binCount(); ++i) {
        if (m_mags[i] > 0) {
            m_mags[i] += m_alpha * (m_dtft.magnitude()[i] - m_mags[i]);
        } else {
            m_mags[i] = m_dtft.magnitude()[i];
        }

        // Calculate SPL.
        m_spls[i] = 10 * log10(m_mags[i] / 1.0);
    }
}

const double *GeneratorSpectrum::frequencies() const { return m_freqs.data(); }

const double *GeneratorSpectrum::magnitudes() const { return m_mags.data(); }

const double *GeneratorSpectrum::magnitudesDb() const { return m_spls.data(); }

int GeneratorSpectrum::binCount() const { return m_dtft.binCount(); }