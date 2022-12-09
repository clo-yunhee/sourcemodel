#include "RosenbergC.h"

#include <boost/math/constants/constants.hpp>

using boost::math::constants::half_pi;
using boost::math::constants::pi;
using boost::math::constants::two_div_pi;

using namespace models;

double RosenbergC::evaluate(double t) {
    static constexpr double T0 = 1;

    double dg;

    if (t < 0 || t > 1) {
        t = -fmod(fabs(t), 1.0);
    }

    if (t <= m_Tp) {
        dg = pi<double>() * m_A / (2 * m_Tp) * std::sin((pi<double>() * t) / m_Tp);
    } else if (t <= m_Tp + m_Tn) {
        dg = -pi<double>() * m_A / (2 * m_Tn) *
             std::sin((half_pi<double>() * (t - m_Tp)) / m_Tn);
    } else {
        dg = 0;
    }
    return dg;
}

bool RosenbergC::fitParameters(const GlottalFlowParameters& params) {
    const double Oq = params.Oq.value();
    const double am = params.am.value();

    m_Tp = am * Oq;
    m_Tn = Oq - m_Tp;
    m_A = two_div_pi<double>() * m_Tn;

    return true;
}

void RosenbergC::updateParameterBounds(GlottalFlowParameters& params) {
    params.Oq.setMin(0.35);
    params.Oq.setMax(0.85);

    params.am.setMin(0.55);
    params.am.setMax(0.9);

    params.Qa.setFixed(0);
}