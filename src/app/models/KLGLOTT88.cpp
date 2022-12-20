#include "KLGLOTT88.h"

using namespace models;

double KLGLOTT88::evaluate(double t) const {
    static constexpr double T0 = 1;

    double dg;

    if (t < 0 || t > 1) {
        t = -fmod(fabs(t), 1.0);
    }

    if (t <= m_Oq * T0) {
        dg = 2.0 * t / m_Oq - 3.0 * t * t / (m_Oq * m_Oq);
    } else {
        dg = 0;
    }
    return dg;
}

bool KLGLOTT88::fitParameters(const GlottalFlowParameters& params) {
    m_Oq = params.Oq.value();
    return true;
}

void KLGLOTT88::updateParameterBounds(GlottalFlowParameters& params) {
    params.Oq.setMin(0.35);
    params.Oq.setMax(0.85);

    params.am.setFixed(0.667);

    params.Qa.setFixed(0);
}