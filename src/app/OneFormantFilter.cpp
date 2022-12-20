#include "OneFormantFilter.h"

#include <boost/math/constants/constants.hpp>
#include <boost/math/special_functions/cos_pi.hpp>
#include <boost/math/special_functions/sin_pi.hpp>
#include <cmath>

using namespace boost::math::double_constants;
using boost::math::cos_pi;
using boost::math::sin_pi;

OneFormantFilter::OneFormantFilter(const double fc, const double gain, const double bw,
                                   const double fs)
    : m_fs(fs),
      m_fc(fc),
      m_q(fc / bw),
      m_bw(bw),
      m_gain(gain),
      m_fcMult(1),
      m_qMult(1),
      m_gainOff(0) {}

double OneFormantFilter::sampleRate() const { return m_fs; }

void OneFormantFilter::setSampleRate(const double fs) { m_fs = fs; }

double OneFormantFilter::frequency() const { return m_fc; }

void OneFormantFilter::setFrequency(const double fc) {
    m_fc = std::min(fc, m_fs / 2);
    m_bw = m_fc / m_q;
}

double OneFormantFilter::quality() const { return m_q; }

void OneFormantFilter::setQuality(const double q) {
    m_q = q;
    m_bw = m_fc / q;
}

double OneFormantFilter::bandwidth() const { return m_bw; }

void OneFormantFilter::setBandwidth(const double bw) {
    m_bw = bw;
    m_q = m_fc / bw;
}

double OneFormantFilter::gain() const { return m_gain; }

void OneFormantFilter::setGain(const double gain) { m_gain = gain; }

double OneFormantFilter::frequencyMultiplier() const { return m_fcMult; }

void OneFormantFilter::setFrequencyMultiplier(const double fcMult) { m_fcMult = fcMult; }

double OneFormantFilter::qualityMultiplier() const { return m_qMult; }

void OneFormantFilter::setQualityMultiplier(const double qMult) { m_qMult = qMult; }

double OneFormantFilter::gainOffset() const { return m_gainOff; }

void OneFormantFilter::setGainOffset(const double gainOff) { m_gainOff = gainOff; }

void OneFormantFilter::update() {
    const double linGain = std::pow(10.0, 0.05 * (m_gain + m_gainOff));
    const double wc = (m_fc * m_fcMult) * two_pi;
    const double a1 = 1.0 / (m_q * m_qMult);
    const double b2 = 0.0;
    const double b1 = linGain;
    const double b0 = 0.0;

    const double c = 1.0 / tan(wc * 0.5 / m_fs);
    const double csq = c * c;
    const double d = 1 + a1 * c + csq;

    const double _b0 = (b0 + b1 * c + b2 * csq) / d;
    const double _b1 = 2.0 * (b0 - b2 * csq) / d;
    const double _b2 = (b0 - b1 * c + b2 * csq) / d;
    const double _a1 = 2 * (1 - csq) / d;
    const double _a2 = (1 - a1 * c + csq) / d;

    m_biquad.update(_b0, _b1, _b2, _a1, _a2);
}

double OneFormantFilter::tick(const double x) { return m_biquad.tick(x); }
