#ifndef SOURCEMODEL__FILTER_SPECTRUM_H
#define SOURCEMODEL__FILTER_SPECTRUM_H

#include <array>
#include <vector>

#include "math/DTFT.h"
#include "math/utils.h"

class FilterSpectrum {
   public:
    FilterSpectrum();

    void setSize(int nfft);
    void setSampleRate(Scalar fs);

    void update(const std::vector<std::array<Scalar, 6>>& sos);

    const Scalar* frequencies() const;
    const Scalar* magnitudes() const;
    const Scalar* magnitudesDb() const;
    int           binCount() const;

   private:
    void calculateOneSection(const std::array<Scalar, 6>& section);

    int    m_nfft;
    int    m_binCount;
    Scalar m_fs;

    std::vector<Scalar> m_freqs;
    std::vector<Scalar> m_sectionMag;
    std::vector<Scalar> m_mags;
    std::vector<Scalar> m_spls;

    std::vector<Scalar> m_data;
    DTFT<Scalar>        m_dtft;
};

#endif  // SOURCEMODEL__FILTER_SPECTRUM_H