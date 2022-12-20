#include "RPlusPlus.h"

#include "../math/utils.h"

using namespace models;

double RPlusPlus::evaluate(double t) const {
    static constexpr double T0 = 1;

    double dg;

    if (t < 0 || t > 1) {
        t = -fmod(fabs(t), 1.0);
    }

    if (t <= m_Te * T0) {
        dg = 4 * m_K * t * (m_Tp - t) * (m_Tx - t);
    } else if (m_Ta > 1e-3) {
        dg = m_dgTe * (exp(-(t - m_Te) / m_Ta) - exp(-(T0 - m_Te) / m_Ta)) /
             (1 - exp(-(T0 - m_Te) / m_Ta));
    } else {
        dg = 0;
    }
    return dg;
}

bool RPlusPlus::fitParameters(const GlottalFlowParameters& params) {
    static constexpr double E = 1;
    static constexpr double T0 = 1;

    const double Oq = params.Oq.value();
    const double am = params.am.value();
    const double Qa = params.Qa.value();

    const double Te = Oq * T0;
    const double Tp = (am > 0.5 ? am : 0.5 + 1e-6) * Oq * T0;
    const double Ta = Qa * (1 - Oq) * T0;

    double Tx;
    if (Ta > 1e-3) {
        const double D = 1 - ((T0 - Te) / Ta) / (exp((T0 - Te) / Ta) - 1);
        Tx = Te * (1 - (.5 * Te * Te - Te * Tp) /
                           (2 * Te * Te - 3 * Te * Tp + 6 * Ta * (Te - Tp) * D));
    } else {
        Tx = Te * (3 * Te - 4 * Tp) / (2 * (2 * Te - 3 * Tp));
    }

    m_K = -E / (4 * Te * (Tp - Te) * (Tx - Te));
    m_Te = Te;
    m_Tp = Tp;
    m_Ta = Ta;
    m_Tx = Tx;

    m_dgTe = 4 * m_K * m_Te * (m_Tp - m_Te) * (m_Tx - m_Te);

    return true;
}

void RPlusPlus::updateParameterBounds(GlottalFlowParameters& params) {
    params.Oq.setMin(0.35);
    params.Oq.setMax(0.85);

    params.Qa.setMin(0);
    params.Qa.setMax(0.9);

    params.am.setMin(0.55);

    static constexpr double T0 = 1;
    const double            Te = params.Oq.value();
    const double            Ta = params.Qa.value() * (1 - Te);

    if (Ta > 1e-3) {
        const double D = 1 - ((T0 - Te) / Ta) / (exp((T0 - Te) / Ta) - 1);
        const double maxAm = .75 * (Te + 4 * Ta * D) / (Te + 3 * Ta * D);

        // Round to 3rd decimal digit.
        params.am.setMax(std::round(maxAm * 1e3) / 1e3);
    } else {
        params.am.setMax(.75);
    }
}