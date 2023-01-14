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
    FormantGenerator(const AudioTime& time, const std::vector<Scalar>& input);

    static constexpr int kNumFormants = 5;

    Scalar frequency(int k) const;
    void   setFrequency(int k, Scalar Fk);

    Scalar bandwidth(int k) const;
    void   setBandwidth(int k, Scalar Bk);

    const FilterSpectrum& spectrum() const;
    FilterSpectrum&       spectrum();

    void updateSpectrumIfNeeded();

   protected:
    void fillInternalBuffer(std::vector<Scalar>& out) override;

   private:
    void updateSpectrum();

    FilterSpectrum   m_spectrum;
    std::atomic_bool m_mustRegenSpectrum;

    // Target value for each parameter.
    std::array<Scalar, kNumFormants> m_targetF;
    std::array<Scalar, kNumFormants> m_targetB;

    // NFParam for each parameter.
    std::array<std::shared_ptr<nativeformat::param::Param>, kNumFormants> m_F;
    std::array<std::shared_ptr<nativeformat::param::Param>, kNumFormants> m_B;

    // One filter per formant.
    std::array<OneFormantFilter, kNumFormants> m_filters;
    // One extra filter for lip radiation.
    Scalar m_lipRadiationCoeff;   // leaking integrator coeff
    Scalar m_lipRadiationMemory;  // last input.

    const std::vector<Scalar>& m_input;
};

#endif  // SOURCEMODEL__FORMANT_GENERATOR_H