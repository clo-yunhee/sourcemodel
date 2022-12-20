#ifndef SOURCEMODEL__GENERATOR_SPECTRUM_H
#define SOURCEMODEL__GENERATOR_SPECTRUM_H

#include <vector>

#include "math/DTFT.h"

class BufferedGenerator;

class GeneratorSpectrum {
   public:
    GeneratorSpectrum(BufferedGenerator* initialGenerator);

    void setTransformSize(int nfft);
    void setSmoothing(double alpha);
    void setSampleRate(double fs);

    void setGenerator(BufferedGenerator* generator);

    void update();

    const double* frequencies() const;
    const double* magnitudes() const;
    const double* magnitudesDb() const;
    int           binCount() const;

   private:
    int    m_nfft;
    double m_alpha;
    double m_fs;

    BufferedGenerator* m_generator;

    std::vector<double> m_window;
    std::vector<double> m_values;

    std::vector<double> m_freqs;
    std::vector<double> m_mags;
    std::vector<double> m_spls;

    DTFT m_dtft;
};

#endif  // SOURCEMODEL__GENERATOR_SPECTRUM_H