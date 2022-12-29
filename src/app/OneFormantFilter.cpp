#include "OneFormantFilter.h"

#include <boost/math/constants/constants.hpp>
#include <boost/math/special_functions/cos_pi.hpp>
#include <boost/math/special_functions/sin_pi.hpp>
#include <cmath>

using namespace boost::math::double_constants;
using boost::math::cos_pi;
using boost::math::sin_pi;

OneFormantFilter::OneFormantFilter(const double fc, const double bw, const double fs)
    : m_fs(fs), m_fc(fc), m_q(fc / bw), m_bw(bw), m_fcMult(1), m_qMult(1) {}

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

double OneFormantFilter::frequencyMultiplier() const { return m_fcMult; }

void OneFormantFilter::setFrequencyMultiplier(const double fcMult) { m_fcMult = fcMult; }

double OneFormantFilter::qualityMultiplier() const { return m_qMult; }

void OneFormantFilter::setQualityMultiplier(const double qMult) { m_qMult = qMult; }

void OneFormantFilter::update() {
    const double r = exp(-pi * m_bw / m_fs);

    const double cosTheta = cos_pi(2 * m_fc / m_fs);
    const double sinTheta = sin_pi(2 * m_fc / m_fs);

    const double b0 = 1;
    const double b1 = 0;
    const double b2 = 0;

    const double a1 = -2 * r * cosTheta;
    const double a2 = r * r;

    // Gain at DC is when z = +1.
    const double gDC = (b0 + b1 + b2) / (1 + a1 + a2);

    m_biquad.update(b0 / gDC, b1 / gDC, b2 / gDC, a1, a2);
    m_coefs = {b0 / gDC, b1 / gDC, b2 / gDC, 1.0, a1, a2};
}

double OneFormantFilter::tick(const double x) { return m_biquad.tick(x); }

const std::array<double, 6>& OneFormantFilter::getBiquadCoefficients() const {
    return m_coefs;
}