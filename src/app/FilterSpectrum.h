#ifndef SOURCEMODEL__FILTER_SPECTRUM_H
#define SOURCEMODEL__FILTER_SPECTRUM_H

#include <array>
#include <vector>

#include "math/DTFT.h"

class FilterSpectrum {
   public:
    FilterSpectrum();

    void setSize(int nfft);
    void setSampleRate(double fs);

    void update(const std::vector<std::array<double, 6>>& sos);

    const double* frequencies() const;
    const double* magnitudes() const;
    const double* magnitudesDb() const;
    int           binCount() const;

   private:
    void calculateOneSection(const std::array<double, 6>& section);

    int    m_nfft;
    int    m_binCount;
    double m_fs;

    std::vector<double> m_freqs;
    std::vector<double> m_sectionMag;
    std::vector<double> m_mags;
    std::vector<double> m_spls;

    std::vector<double> m_data;
    DTFT                m_dtft;
};

#endif  // SOURCEMODEL__FILTER_SPECTRUM_H