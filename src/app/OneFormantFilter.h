#ifndef SOURCEMODEL__ONEFORMANTFILTER_H
#define SOURCEMODEL__ONEFORMANTFILTER_H

#include <array>

#include "math/filters/SVFBiquad.h"

class OneFormantFilter {
   public:
    OneFormantFilter(double fc = 100, double bw = 50, double fs = 48000);

    double sampleRate() const;
    void   setSampleRate(double fs);

    double frequency() const;
    void   setFrequency(double fc);

    double quality() const;
    void   setQuality(double q);

    double bandwidth() const;
    void   setBandwidth(double bw);

    double frequencyMultiplier() const;
    void   setFrequencyMultiplier(double fcMult);

    double qualityMultiplier() const;
    void   setQualityMultiplier(double qMult);

    void   update();
    double tick(double x);

    const std::array<double, 6>& getBiquadCoefficients() const;

   private:
    double m_fs;
    double m_fc;
    double m_q;
    double m_bw;

    double m_fcMult;
    double m_qMult;

    SVFBiquad m_biquad;

    std::array<double, 6> m_coefs;
};

#endif  // SOURCEMODEL__ONEFORMANTFILTER_H