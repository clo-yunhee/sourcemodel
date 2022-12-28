#ifndef SOURCEMODEL__FORMANT_GENERATOR_H
#define SOURCEMODEL__FORMANT_GENERATOR_H

#include <NFParam/Param.h>

#include <array>

#include "OneFormantFilter.h"
#include "audio/BufferedGenerator.h"

class FormantGenerator : public BufferedGenerator {
   public:
    FormantGenerator(const AudioTime& time, const std::vector<double>& input);

    static constexpr int kNumFormants = 5;

    double frequency(int k) const;
    void   setFrequency(int k, double Fk);

    double bandwidth(int k) const;
    void   setBandwidth(int k, double Bk);

    double gain(int k) const;
    void   setGain(int k, double Gk);

   protected:
    void fillInternalBuffer(std::vector<double>& out) override;

   private:
    // Target value for each parameter.
    std::array<double, kNumFormants> m_targetF;
    std::array<double, kNumFormants> m_targetB;
    std::array<double, kNumFormants> m_targetG;

    // NFParam for each parameter.
    std::array<std::shared_ptr<nativeformat::param::Param>, kNumFormants> m_F;
    std::array<std::shared_ptr<nativeformat::param::Param>, kNumFormants> m_B;
    std::array<std::shared_ptr<nativeformat::param::Param>, kNumFormants> m_G;

    // One filter per formant.
    std::array<OneFormantFilter, kNumFormants> m_filters;

    const std::vector<double>& m_input;
};

#endif  // SOURCEMODEL__FORMANT_GENERATOR_H