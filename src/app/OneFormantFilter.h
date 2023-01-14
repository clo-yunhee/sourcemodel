#ifndef SOURCEMODEL__ONEFORMANTFILTER_H
#define SOURCEMODEL__ONEFORMANTFILTER_H

#include <array>

#include "math/filters/SVFBiquad.h"

class OneFormantFilter {
   public:
    OneFormantFilter(Scalar fc = 100, Scalar bw = 50, Scalar fs = 48000);

    Scalar sampleRate() const;
    void   setSampleRate(Scalar fs);

    Scalar frequency() const;
    void   setFrequency(Scalar fc);

    Scalar quality() const;
    void   setQuality(Scalar q);

    Scalar bandwidth() const;
    void   setBandwidth(Scalar bw);

    Scalar frequencyMultiplier() const;
    void   setFrequencyMultiplier(Scalar fcMult);

    Scalar qualityMultiplier() const;
    void   setQualityMultiplier(Scalar qMult);

    void   update();
    Scalar tick(Scalar x);

    const std::array<Scalar, 6>& getBiquadCoefficients() const;

   private:
    Scalar m_fs;
    Scalar m_fc;
    Scalar m_q;
    Scalar m_bw;

    Scalar m_fcMult;
    Scalar m_qMult;

    SVFBiquad m_biquad;

    std::array<Scalar, 6> m_coefs;
};

#endif  // SOURCEMODEL__ONEFORMANTFILTER_H