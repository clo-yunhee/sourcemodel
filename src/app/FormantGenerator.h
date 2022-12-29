#ifndef SOURCEMODEL__FORMANT_GENERATOR_H
#define SOURCEMODEL__FORMANT_GENERATOR_H

#include <NFParam/Param.h>

#include <array>
#include <atomic>

#include "FilterSpectrum.h"
#include "OneFormantFilter.h"
#include "audio/BufferedGenerator.h"
#include "math/filters/SOSFilter.h"

class FilterSpectrum;

class FormantGenerator : public BufferedGenerator {
   public:
    FormantGenerator(const AudioTime& time, const std::vector<double>& input);

    static constexpr int kNumFormants = 5;

    double frequency(int k) const;
    void   setFrequency(int k, double Fk);

    double bandwidth(int k) const;
    void   setBandwidth(int k, double Bk);

    const FilterSpectrum& spectrum() const;
    FilterSpectrum&       spectrum();

    void updateSpectrumIfNeeded();

   protected:
    void fillInternalBuffer(std::vector<double>& out) override;

   private:
    void updateSpectrum();

    FilterSpectrum   m_spectrum;
    std::atomic_bool m_mustRegenSpectrum;

    // Target value for each parameter.
    std::array<double, kNumFormants> m_targetF;
    std::array<double, kNumFormants> m_targetB;

    // NFParam for each parameter.
    std::array<std::shared_ptr<nativeformat::param::Param>, kNumFormants> m_F;
    std::array<std::shared_ptr<nativeformat::param::Param>, kNumFormants> m_B;

    // One filter per formant.
    std::array<OneFormantFilter, kNumFormants> m_filters;
    // One extra filter for lip radiation.
    double m_lipRadiationCoeff;   // leaking integrator coeff
    double m_lipRadiationMemory;  // last input.

    const std::vector<double>& m_input;
};

#endif  // SOURCEMODEL__FORMANT_GENERATOR_H