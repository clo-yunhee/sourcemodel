#include "FilterSpectrum.h"

#include <boost/math/constants/constants.hpp>
#include <boost/math/tools/rational.hpp>

#include "math/utils.h"

using namespace boost::math::double_constants;

FilterSpectrum::FilterSpectrum() : m_fs(48000) { setSize(1024); }

void FilterSpectrum::setSize(const int nfft) {
    if (m_nfft != nfft) {
        m_nfft = nfft;
        m_dtft.setSampleCount(nfft);
        // Fill data with zeroes
        m_data.resize(m_nfft);
        std::fill(m_data.begin(), m_data.end(), 0.0);
        // Reconstruct frequency array.
        m_binCount = m_dtft.binCount();
        m_freqs.resize(m_binCount);
        m_sectionMag.resize(m_binCount);
        m_mags.resize(m_binCount);
        m_spls.resize(m_binCount);
        for (int i = 0; i < m_binCount; ++i) {
            m_freqs[i] = (i * m_fs * 0.5) / m_binCount;
            m_mags[i] = 0;
            m_spls[i] = -std::numeric_limits<double>::infinity();
        }
    }
}

void FilterSpectrum::setSampleRate(const double fs) {
    if (!fuzzyEquals(m_fs, fs)) {
        m_fs = fs;
        // Reconstruct frequency array.
        for (int i = 0; i < m_binCount; ++i) {
            m_freqs[i] = (i * m_fs * 0.5) / m_binCount;
            m_mags[i] = 0;
            m_spls[i] = -std::numeric_limits<double>::infinity();
        }
    }
}

void FilterSpectrum::update(const std::vector<std::array<double, 6>> &sos) {
    for (int i = 0; i < m_binCount; ++i) {
        m_mags[i] = 1.0;
    }
    for (const auto &sec : sos) {
        calculateOneSection(sec);
    }
    for (int i = 0; i < m_binCount; ++i) {
        m_spls[i] = 10 * log10(m_mags[i]);
    }
}

const double *FilterSpectrum::frequencies() const { return m_freqs.data(); }

const double *FilterSpectrum::magnitudes() const { return m_mags.data(); }

const double *FilterSpectrum::magnitudesDb() const { return m_spls.data(); }

int FilterSpectrum::binCount() const { return m_binCount; }

void FilterSpectrum::calculateOneSection(const std::array<double, 6> &sec) {
    // Calculate for b
    std::copy(sec.begin(), std::next(sec.begin(), 3), m_data.begin());
    m_dtft.updateSamples(m_data.data(), m_nfft);
    std::copy(m_dtft.magnitude(), m_dtft.magnitude() + m_binCount, m_sectionMag.begin());

    // Calculate for a
    std::copy(std::next(sec.begin(), 3), std::next(sec.begin(), 6), m_data.begin());
    m_dtft.updateSamples(m_data.data(), m_nfft);
    for (int i = 0; i < m_binCount; ++i) {
        m_mags[i] *= (m_sectionMag[i] / m_dtft.magnitude()[i]);
    }
}