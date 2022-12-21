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
            const double x = double(i) / double(m_nfft - 1);

            // Flat top window
            constexpr double a0 = 0.21557895;
            constexpr double a1 = 0.41663158;
            constexpr double a2 = 0.277263158;
            constexpr double a3 = 0.083578947;
            constexpr double a4 = 0.006947368;
            m_window[i] = a0 - a1 * cos(two_pi * x) + a2 * cos(2 * two_pi * x) -
                          a3 * cos(3 * two_pi * x) + a4 * cos(4 * two_pi * x);
        }
        // Reconstruct frequency array.
        m_freqs.resize(m_dtft.binCount());
        m_mags.resize(m_dtft.binCount());
        m_spls.resize(m_dtft.binCount());
        for (int i = 0; i < m_dtft.binCount(); ++i) {
            m_freqs[i] = (i * m_fs) / m_nfft;
            m_mags[i] = 0;
            m_spls[i] = -std::numeric_limits<double>::infinity();
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
            m_spls[i] = -std::numeric_limits<double>::infinity();
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
        m_mags[i] = m_dtft.magnitude()[i];

        // Calculate SPL.
        const double newSpl = 10 * log10(m_mags[i]);

        if (std::isinf(m_spls[i])) {
            m_spls[i] = newSpl;
        } else {
            m_spls[i] = m_alpha * newSpl + (1 - m_alpha) * m_spls[i];
        }
    }
}

const double *GeneratorSpectrum::frequencies() const { return m_freqs.data(); }

const double *GeneratorSpectrum::magnitudes() const { return m_mags.data(); }

const double *GeneratorSpectrum::magnitudesDb() const { return m_spls.data(); }

int GeneratorSpectrum::binCount() const { return m_dtft.binCount(); }