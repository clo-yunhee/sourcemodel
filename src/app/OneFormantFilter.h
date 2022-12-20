#ifndef SOURCEMODEL__ONEFORMANTFILTER_H
#define SOURCEMODEL__ONEFORMANTFILTER_H

#include "math/filters/SVFBiquad.h"

class OneFormantFilter {
   public:
    OneFormantFilter(double fc = 100, double gain = 0, double bw = 50, double fs = 48000);

    double sampleRate() const;
    void   setSampleRate(double fs);

    double frequency() const;
    void   setFrequency(double fc);

    double quality() const;
    void   setQuality(double q);

    double bandwidth() const;
    void   setBandwidth(double bw);

    double gain() const;
    void   setGain(double gain);

    double frequencyMultiplier() const;
    void   setFrequencyMultiplier(double fcMult);

    double qualityMultiplier() const;
    void   setQualityMultiplier(double qMult);

    double gainOffset() const;
    void   setGainOffset(double gainOff);

    void   update();
    double tick(double x);

   private:
    double m_fs;
    double m_fc;
    double m_q;
    double m_bw;
    double m_gain;

    double m_fcMult;
    double m_qMult;
    double m_gainOff;

    SVFBiquad m_biquad;
};

#endif  // SOURCEMODEL__ONEFORMANTFILTER_H