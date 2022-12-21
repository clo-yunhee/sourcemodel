#include "RosenbergC.h"

#include <boost/math/constants/constants.hpp>

using namespace boost::math::double_constants;
using namespace models;

double RosenbergC::evaluate(double t) const {
    static constexpr double T0 = 1;

    double dg;

    if (t < 0 || t > 1) {
        t = -fmod(fabs(t), 1.0);
    }

    if (t <= m_Tp) {
        dg = half_pi * m_A / m_Tp * std::sin((pi * t) / m_Tp);
    } else if (t <= m_Tp + m_Tn) {
        dg = -half_pi * m_A / m_Tn * std::sin((half_pi * (t - m_Tp)) / m_Tn);
    } else {
        dg = 0;
    }
    return dg;
}

void RosenbergC::fitParameters(const GlottalFlowParameters& params) {
    const double Oq = params.Oq.value();
    const double am = params.am.value();

    m_Tp = am * Oq;
    m_Tn = Oq - m_Tp;
    m_A = two_div_pi * m_Tn;
}

void RosenbergC::updateParameterBounds(GlottalFlowParameters& params) {
    params.Oq.setMin(0.35);
    params.Oq.setMax(0.85);

    params.am.setMin(0.55);
    params.am.setMax(0.9);

    params.Qa.setFixed(0);
}