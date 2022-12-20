#ifndef SOURCEMODEL__MATH_DTFT_H
#define SOURCEMODEL__MATH_DTFT_H

#include <fftw3.h>

#include <complex>
#include <memory>
#include <vector>

class DTFT {
   public:
    DTFT();

    void setSampleCount(int sampleCount);
    int  sampleCount() const;
    int  binCount() const;

    void updateSamples(const double *samples, int sampleCount);

    const double *magnitude() const;

   private:
    struct PlanDeleter {
        void operator()(fftw_plan plan) { fftw_destroy_plan(plan); }
    };

    struct DataDeleter {
        void operator()(void *p) { fftw_free(p); }
    };

    std::unique_ptr<std::remove_pointer_t<fftw_plan>, PlanDeleter> m_fftPlan;

    int                                                  m_sampleCount;
    std::unique_ptr<double[], DataDeleter>               m_input;
    std::unique_ptr<std::complex<double>[], DataDeleter> m_output;

    std::vector<double> m_mag;
};

#endif  // SOURCEMODEL__MATH_DTFT_H