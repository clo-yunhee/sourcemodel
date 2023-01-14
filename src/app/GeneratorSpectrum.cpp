#include "GeneratorSpectrum.h"

#include <boost/math/constants/constants.hpp>
#include <boost/math/special_functions/cos_pi.hpp>
#include <boost/math/special_functions/sin_pi.hpp>

#include "audio/BufferedGenerator.h"
#include "math/utils.h"

using namespace boost::math::constants;
using boost::math::cos_pi;
using boost::math::sin_pi;

GeneratorSpectrum::GeneratorSpectrum(BufferedGenerator *initialGenerator)
    : m_generator(initialGenerator), m_updatePeriod(512) {
    // Preallocate for max NFFT = 32768
    constexpr int maxNfft = 32768;
    constexpr int maxNbins = maxNfft / 2 + 1;
    m_smoothedMags.reserve(maxNbins);
    m_window.reserve(maxNfft);
    m_values.reserve(maxNfft);
    m_freqs.reserve(maxNbins);
    m_mags.reserve(maxNbins);
    m_spls.reserve(maxNbins);
}

int GeneratorSpectrum::transformSize() const { return m_dtft.sampleCount(); }

void GeneratorSpectrum::setTransformSize(const int nfft) {
    if (m_nfft != nfft) {
        m_nfft = nfft;
        m_dtft.setSampleCount(nfft);
        m_generator->setBufferLength(nfft);
        constructWindow();
        constructFrequencyArray();
        constructSmoothingKernel();
    }
}

Scalar GeneratorSpectrum::responseTime() const { return m_responseTime; }

void GeneratorSpectrum::setResponseTime(const Scalar responseTime) {
    if (!fuzzyEquals(m_responseTime, responseTime)) {
        m_responseTime = responseTime;
        constructSmoothingKernel();
    }
}

void GeneratorSpectrum::setSampleRate(const Scalar fs) {
    if (!fuzzyEquals(m_fs, fs)) {
        m_fs = fs;
        constructFrequencyArray();
        constructSmoothingKernel();
        m_lastUpdate = 0;
    }
}

void GeneratorSpectrum::setGenerator(BufferedGenerator *generator) {
    m_generator = generator;
    generator->setBufferLength(m_nfft);
}

void GeneratorSpectrum::update() {
    if (m_generator->hasEnoughSamplesSince(m_lastUpdate, m_updatePeriod)) {
        m_lastUpdate = m_generator->copyBufferTo(m_values);

        for (int i = 0; i < m_nfft; ++i) {
            m_values[i] *= m_window[i];
        }

        m_dtft.updateSamples(m_values.data(), m_nfft);

        // Copy magnitude.
        for (int i = 0; i < m_binCount; ++i) {
            m_mags[i] = m_dtft.magnitude()[i];
        }

        // Apply smoothing.
        for (int i = 0; i < m_binCount; ++i) {
            m_smoothedMags[i] = m_alpha * m_mags[i] + (1 - m_alpha) * m_smoothedMags[i];
        }

        // Calculate SPL in dB.
        for (int i = 0; i < m_binCount; ++i) {
            m_spls[i] = 10 * std::log10(m_smoothedMags[i]);
        }
    }
}

const Scalar *GeneratorSpectrum::frequencies() const { return m_freqs.data(); }

const Scalar *GeneratorSpectrum::magnitudes() const { return m_smoothedMags.data(); }

const Scalar *GeneratorSpectrum::magnitudesDb() const { return m_spls.data(); }

int GeneratorSpectrum::binCount() const { return m_dtft.binCount(); }

void GeneratorSpectrum::constructWindow() {
    m_window.resize(m_nfft);
    m_values.resize(m_nfft);

#ifdef __EMSCRIPTEN__
    m_window = windows::blackmanHarris<Scalar>(m_nfft, true);
#else
    m_window = windows::chebwin<Scalar>(m_nfft, 320, true);
#endif
}

void GeneratorSpectrum::constructFrequencyArray() {
    m_binCount = m_dtft.binCount();
    m_freqs.resize(m_binCount);
    m_mags.resize(m_binCount);
    m_spls.resize(m_binCount);
    m_smoothedMags.resize(m_binCount);
    for (int i = 0; i < m_binCount; ++i) {
        m_freqs[i] = (i * m_fs) / m_nfft;
        m_mags[i] = 0;
        m_spls[i] = -std::numeric_limits<Scalar>::infinity();
        m_smoothedMags[i] = 0;
    }
}

void GeneratorSpectrum::constructSmoothingKernel() {
    // An exponential smoothing filter is a simple IIR filter
    // s_i = alpha*x_i + (1-alpha)*s_{i-1}
    // We compute alpha so that the N most recent samples represent w% of the output
    constexpr Scalar w = 0.65_f;
    const int        n = (m_responseTime * m_fs) / m_nfft;
    const int        N = m_updatePeriod;
    m_alpha = 1.0_f - std::pow(1.0_f - w, 1.0_f / (n + 1));
}
