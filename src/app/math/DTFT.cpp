#include "DTFT.h"

#ifdef __EMSCRIPTEN__
    #define FFTW_SETTING FFTW_ESTIMATE
#else
    #define FFTW_SETTING FFTW_MEASURE
#endif

DTFT::DTFT() : m_sampleCount(0) {}

void DTFT::setSampleCount(int sampleCount) {
    if (m_sampleCount != sampleCount) {
        m_sampleCount = sampleCount;
        m_input.reset((double *)fftw_alloc_real(sampleCount));
        m_output.reset((std::complex<double> *)fftw_alloc_complex(sampleCount / 2 + 1));
        m_mag.resize(sampleCount / 2 + 1);
        m_fftPlan.reset(fftw_plan_dft_r2c_1d(
            sampleCount, m_input.get(), (fftw_complex *)m_output.get(), FFTW_SETTING));
    }
}

int DTFT::sampleCount() const { return m_sampleCount; }

int DTFT::binCount() const { return m_sampleCount / 2 + 1; }

void DTFT::updateSamples(const double *samples, const int sampleCount) {
    if (m_sampleCount != sampleCount) {
        setSampleCount(sampleCount);
    }

    std::copy(&samples[0], &samples[sampleCount], &m_input[0]);
    fftw_execute(m_fftPlan.get());

    // Calculate magnitude.
    for (int i = 0; i < sampleCount / 2 + 1; ++i) {
        m_mag[i] = std::abs(m_output[i]);
    }
}

const double *DTFT::magnitude() const { return m_mag.data(); }