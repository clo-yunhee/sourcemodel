#ifndef SOURCEMODEL__MATH_DTFT_H
#define SOURCEMODEL__MATH_DTFT_H

#include <complex>
#include <fftw3cxx.hh>
#include <vector>

#ifdef __EMSCRIPTEN__
    #define FFTW_SETTING FFTW_ESTIMATE
#else
    #define FFTW_SETTING FFTW_ESTIMATE
#endif

template <typename T>
class DTFT {
   public:
    DTFT() : m_sampleCount(0), m_input(nullptr), m_output(nullptr) {}

    ~DTFT() {}

    void setSampleCount(const int sampleCount) {
        if (m_sampleCount != sampleCount) {
            cleanup();
            m_sampleCount = sampleCount;
            m_input = (T *)fftw3cxx::malloc<T>(sampleCount * sizeof(T));
            m_output = (std::complex<T> *)fftw3cxx::malloc<T>((sampleCount / 2 + 1) *
                                                              sizeof(std::complex<T>));
            m_mag.resize(sampleCount / 2 + 1);
            m_plan = fftw3cxx::plan<T>::plan_dft_r2c_1d(sampleCount, m_input, m_output,
                                                        FFTW_SETTING);
        }
    }

    int sampleCount() const { return m_sampleCount; }

    int binCount() const { return m_sampleCount / 2 + 1; }

    void updateSamples(const T *samples, int sampleCount) {
        if (m_sampleCount != sampleCount) {
            setSampleCount(sampleCount);
        }

        std::copy(&samples[0], &samples[sampleCount], &m_input[0]);
        m_plan.execute();

        for (int i = 0; i < sampleCount / 2 + 1; ++i) {
            m_mag[i] = std::abs(m_output[i]) / sampleCount;
        }
    }

    const T *magnitude() const { return m_mag.data(); }

   private:
    void cleanup() {
        if (m_input != nullptr) {
            fftw3cxx::free<T>(m_input);
            m_input = nullptr;
        }
        if (m_output != nullptr) {
            fftw3cxx::free<T>(m_output);
            m_output = nullptr;
        }
    }

    fftw3cxx::plan<T> m_plan;

    int              m_sampleCount;
    T               *m_input;
    std::complex<T> *m_output;

    std::vector<T> m_mag;
};

#endif  // SOURCEMODEL__MATH_DTFT_H