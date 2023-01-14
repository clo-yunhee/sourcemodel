#include "OneFormantFilter.h"

#include <boost/math/constants/constants.hpp>
#include <boost/math/special_functions/cos_pi.hpp>
#include <boost/math/special_functions/sin_pi.hpp>
#include <cmath>

using namespace boost::math::constants;
using boost::math::cos_pi;
using boost::math::sin_pi;

OneFormantFilter::OneFormantFilter(const Scalar fc, const Scalar bw, const Scalar fs)
    : m_fs(fs), m_fc(fc), m_q(fc / bw), m_bw(bw), m_fcMult(1), m_qMult(1) {}

Scalar OneFormantFilter::sampleRate() const { return m_fs; }

void OneFormantFilter::setSampleRate(const Scalar fs) { m_fs = fs; }

Scalar OneFormantFilter::frequency() const { return m_fc; }

void OneFormantFilter::setFrequency(const Scalar fc) {
    m_fc = std::min(fc, m_fs / 2);
    m_bw = m_fc / m_q;
}

Scalar OneFormantFilter::quality() const { return m_q; }

void OneFormantFilter::setQuality(const Scalar q) {
    m_q = q;
    m_bw = m_fc / q;
}

Scalar OneFormantFilter::bandwidth() const { return m_bw; }

void OneFormantFilter::setBandwidth(const Scalar bw) {
    m_bw = bw;
    m_q = m_fc / bw;
}

Scalar OneFormantFilter::frequencyMultiplier() const { return m_fcMult; }

void OneFormantFilter::setFrequencyMultiplier(const Scalar fcMult) { m_fcMult = fcMult; }

Scalar OneFormantFilter::qualityMultiplier() const { return m_qMult; }

void OneFormantFilter::setQualityMultiplier(const Scalar qMult) { m_qMult = qMult; }

void OneFormantFilter::update() {
    const Scalar r = std::exp(-pi<Scalar>() * m_bw / m_fs);

    const Scalar cosTheta = cos_pi(2 * m_fc / m_fs);
    const Scalar sinTheta = sin_pi(2 * m_fc / m_fs);

    const Scalar b0 = 1;
    const Scalar b1 = 0;
    const Scalar b2 = 0;

    const Scalar a1 = -2 * r * cosTheta;
    const Scalar a2 = r * r;

    // Gain at DC is when z = +1.
    const Scalar gDC = (b0 + b1 + b2) / (1 + a1 + a2);

    m_biquad.update(b0 / gDC, b1 / gDC, b2 / gDC, a1, a2);
    m_coefs = {b0 / gDC, b1 / gDC, b2 / gDC, 1.0_f, a1, a2};
}

Scalar OneFormantFilter::tick(const Scalar x) { return m_biquad.tick(x); }

const std::array<Scalar, 6>& OneFormantFilter::getBiquadCoefficients() const {
    return m_coefs;
}