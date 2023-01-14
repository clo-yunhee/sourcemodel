#include "RPlusPlus.h"

#include <boost/math/constants/constants.hpp>

#include "../math/utils.h"

using namespace boost::math::constants;
using namespace models;

inline constexpr Scalar four_thirds = 1.0_f + third<Scalar>();

Scalar RPlusPlus::evaluate(Scalar t) const {
    static constexpr Scalar T0 = 1;

    Scalar dg;

    if (t < 0 || t > 1) {
        t = -std::fmod(std::abs(t), 1.0_f);
    }

    if (t <= m_Te * T0) {
        dg = 4 * m_K * t * (m_Tp - t) * (m_Tx - t);
    } else if (m_Ta > 1e-6) {
        dg = m_dgTe * (std::exp(-(t - m_Te) / m_Ta) - m_expT0TeTa) / (1 - m_expT0TeTa);
    } else {
        dg = 0;
    }
    return dg;
}

Scalar RPlusPlus::evaluateAntiderivative(Scalar t) const {
    static constexpr Scalar T0 = 1;

    Scalar g;

    if (t < 0 || t > 1) {
        t = -std::fmod(std::abs(t), 1.0_f);
    }

    if (t <= m_Te * T0) {
        g = m_K * t * t * (t * t - four_thirds * t * (m_Tp + m_Tx) + 2 * m_Tp * m_Tx);
    } else if (m_Ta > 1e-6) {
        g = m_gTe + m_Ta * m_dgTe *
                        (1.0_f - std::exp(-(t - m_Te) / m_Ta) -
                         ((t - m_Te) / m_Ta) * m_expT0TeTa) /
                        (1 - m_expT0TeTa);
    } else {
        g = 0;
    }

    return g;
}

void RPlusPlus::fitParameters(const GlottalFlowParameters& params) {
    static constexpr Scalar E = 1;
    static constexpr Scalar T0 = 1;

    const Scalar Oq = params.Oq.value();
    const Scalar am = params.am.value();
    const Scalar Qa = params.Qa.value();

    const Scalar Te = Oq * T0;
    const Scalar Tp = (am > 0.5 ? am : 0.5 + 1e-6) * Oq * T0;
    const Scalar Ta = Qa * (1 - Oq) * T0;

    Scalar Tx;
    if (Ta > 1e-6) {
        const Scalar D = 1 - ((T0 - Te) / Ta) / (exp((T0 - Te) / Ta) - 1);
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
    m_gTe = m_K * m_Te * m_Te *
            (m_Te * m_Te - four_thirds * m_Te * (m_Tp + m_Tx) + 2 * m_Tp * m_Tx);

    m_expT0TeTa = std::exp(-(T0 - m_Te) / m_Ta);
}

void RPlusPlus::updateParameterBounds(GlottalFlowParameters& params) {
    params.Oq.setMin(0.35_f);
    params.Oq.setMax(0.85_f);

    params.Qa.setMin(0_f);
    params.Qa.setMax(0.9_f);

    params.am.setMin(0.55_f);

    static constexpr Scalar T0 = 1;
    const Scalar            Te = params.Oq.value();
    const Scalar            Ta = params.Qa.value() * (1 - Te);

    if (Ta > 1e-3) {
        const Scalar D = 1 - ((T0 - Te) / Ta) / (exp((T0 - Te) / Ta) - 1);
        const Scalar maxAm = .75_f * (Te + 4 * Ta * D) / (Te + 3 * Ta * D);

        // Round to 3rd decimal digit.
        params.am.setMax(std::round(maxAm * 1e3_f) / 1e3_f);
    } else {
        params.am.setMax(.75_f);
    }
}