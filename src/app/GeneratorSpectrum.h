#ifndef SOURCEMODEL__GENERATOR_SPECTRUM_H
#define SOURCEMODEL__GENERATOR_SPECTRUM_H

#include <vector>

#include "math/DTFT.h"
#include "math/utils.h"
#include "math/windows.h"

class BufferedGenerator;

class GeneratorSpectrum {
   public:
    GeneratorSpectrum(BufferedGenerator* initialGenerator);

    int  transformSize() const;
    void setTransformSize(int nfft);

    void setSampleRate(Scalar fs);

    Scalar responseTime() const;
    void   setResponseTime(Scalar responseTime);

    void setGenerator(BufferedGenerator* generator);

    void update();

    const Scalar* frequencies() const;
    const Scalar* magnitudes() const;
    const Scalar* magnitudesDb() const;
    int           binCount() const;

   private:
    void constructWindow();
    void constructFrequencyArray();
    void constructSmoothingKernel();

    int    m_nfft;
    Scalar m_fs;
    Scalar m_responseTime;

    BufferedGenerator* m_generator;
    uint64_t           m_lastUpdate;
    int                m_updatePeriod;

    // Smoothing kernel.
    Scalar              m_alpha;
    std::vector<Scalar> m_smoothedMags;

    std::vector<Scalar> m_window;
    std::vector<Scalar> m_values;

    int                 m_binCount;
    std::vector<Scalar> m_freqs;
    std::vector<Scalar> m_mags;
    std::vector<Scalar> m_spls;

    DTFT<Scalar> m_dtft;
};

#endif  // SOURCEMODEL__GENERATOR_SPECTRUM_H