#include "RosenbergC.h"

#include <boost/math/constants/constants.hpp>
#include <boost/math/special_functions/cos_pi.hpp>
#include <boost/math/special_functions/sin_pi.hpp>

using namespace boost::math::constants;
using namespace models;
using boost::math::cos_pi;
using boost::math::sin_pi;

Scalar RosenbergC::evaluate(Scalar t) const {
    Scalar dg;

    if (t < 0 || t > 1) {
        t = -std::fmod(std::abs(t), 1.0_f);
    }

    if (t <= m_Tp) {
        dg = half_pi<Scalar>() * m_A / m_Tp * sin_pi(t / m_Tp);
    } else if (t <= m_Tp + m_Tn) {
        dg = -half_pi<Scalar>() * m_A / m_Tn * sin_pi(0.5_f * (t - m_Tp) / m_Tn);
    } else {
        dg = 0;
    }
    return dg;
}

Scalar RosenbergC::evaluateAntiderivative(Scalar t) const {
    Scalar g;

    if (t < 0 || t > 1) {
        t = -std::fmod(std::abs(t), 1.0_f);
    }

    if (t <= m_Tp) {
        g = m_A / 2.0_f * (1.0_f - cos_pi(t / m_Tp));
    } else if (t <= m_Tp + m_Tn) {
        g = m_A * cos_pi(0.5_f * (t - m_Tp) / m_Tn);
    } else {
        g = 0;
    }
    return g;
}

void RosenbergC::fitParameters(GlottalFlowParameters& params) {
    const Scalar Oq = params.Oq.value();
    const Scalar am = params.am.value();

    m_Tp = am * Oq;
    m_Tn = Oq - m_Tp;
    m_A = two_div_pi<Scalar>() * m_Tn;
}

void RosenbergC::updateParameterBounds(GlottalFlowParameters& params) {
    params.Oq.setMin(0.35_f);
    params.Oq.setMax(0.85_f);

    params.am.setMin(0.55_f);
    params.am.setMax(0.9_f);

    params.Qa.setFixed(0_f);
}