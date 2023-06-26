#include "KLGLOTT88.h"

#include <cmath>

using namespace models;

Scalar KLGLOTT88::evaluate(Scalar t) const {
    Scalar dg;

    if (t < 0 || t > 1) {
        t = -std::fmod(std::abs(t), 1.0_f);
    }

    if (t <= m_Oq) {
        dg = 2.0 * t / m_Oq - 3.0 * t * t / (m_Oq * m_Oq);
    } else {
        dg = 0;
    }
    return dg;
}

Scalar KLGLOTT88::evaluateAntiderivative(Scalar t) const {
    Scalar g;

    if (t < 0 || t > 1) {
        t = -std::fmod(std::abs(t), 1.0_f);
    }

    if (t <= m_Oq) {
        g = t * t / (m_Oq * m_Oq) - std::pow(t, 3.0_f) / std::pow(m_Oq, 3.0_f);
    } else {
        g = 0;
    }
    return g;
}

void KLGLOTT88::fitParameters(GlottalFlowParameters& params) { m_Oq = params.Oq.value(); }

void KLGLOTT88::updateParameterBounds(GlottalFlowParameters& params) {
    params.Oq.setMin(0.35_f);
    params.Oq.setMax(0.85_f);

    params.am.setFixed(0.667_f);

    params.Qa.setFixed(0_f);
}