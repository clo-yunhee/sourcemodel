#include "LFModel.h"

#include <array>

#include "fzero.h"

double LFModel::evaluate(double t) {
    constexpr double T0 = 1;
    double           dg;

    if (t <= m_Te) {
        dg = -m_Ee * exp(m_alpha * (t - m_Te)) * sin((M_PI * t) / m_Tp) /
             sin((M_PI * m_Te) / m_Tp);
    } else if (m_epsilon <= 1e5) {
        dg = -m_Ee / (m_epsilon * m_Ta) *
             (exp(-m_epsilon * (t - m_Te)) - exp(-m_epsilon * (T0 - m_Te)));
    } else {
        dg = 0;
    }
    return dg;
}

bool LFModel::fitParameters(const GlottalFlowParameters& params) {
    constexpr double E0 = 1;
    constexpr double T0 = 1;

    constexpr double precision = std::numeric_limits<double>::epsilon();

    constexpr double tolEpsZero = 1e-4;
    constexpr double tolEpsilon = 1e-12;
    constexpr double tolAlpha = 1e-12;

    const double Oq = params.Oq.value();
    const double am = params.am.value();
    const double Qa = params.Qa.value();

    const double Te = Oq * T0;
    const double Tp = (am > 0.5 ? am : 0.5) * Oq * T0;
    const double Ta = Qa * (1 - Oq) * T0;

    double epsilon;
    double alpha;

    // If Ta is small enough we can skip this and just assign a big value to epsilon.
    if (Ta > precision) {
        // epsilon is expressed by an implicit equation
        const auto fn_e = [=](double e) { return 1.0 - exp(-e * (T0 - Te)) - e * Ta; };
        epsilon = fzero(1.0 / (Ta * (1 + precision)), fn_e, precision);

        // If epsilon did not solve to anything revert the param change.
        if (std::isnan(epsilon)) return false;
    } else {
        epsilon = std::numeric_limits<double>::max();
    }

    // alpha is expressed by another implicit equation
    constexpr double PI_SQUARE = 9.869604401089359;
    const double     wg = M_PI / Tp;
    const double     wg2 = (PI_SQUARE) / (Tp * Tp);
    const double     sinwgTe = sin((M_PI * Te) / Tp);
    const double     coswgTe = cos((M_PI * Te) / Tp);
    const double     wgsinwgTe = wg / sinwgTe;

    double A = (T0 - Te) / (exp(epsilon * (T0 - Te)) - 1) - 1 / epsilon;

    const auto fn_a = [=](double a) {
        return 1.0 / (a * a + wg2) * ((exp(-a * Te) - coswgTe) * wgsinwgTe + a) - A;
    };

    // Find the first interval with a zero crossing
    std::array ints{-1e20, -1e9, -1e8, -1e7, -1e6, -1e5, -1e4, -1e3, -1e2, -1e1, 0.0,
                    1e1,   1e2,  1e3,  1e4,  1e5,  1e6,  1e7,  1e8,  1e9,  1e20};
    double     fa, fb;
    double     a, b;

    for (int i = 0; i < ints.size() - 1; ++i) {
        a = ints[i];
        fa = fn_a(a);

        b = ints[i + 1];
        fb = fn_a(b);

        if (sgn(fa) * sgn(fb) <= 0) break;
    }

    b = ints[0];
    fb = fn_a(b);
    for (int i = 1; i < ints.size(); ++i) {
        a = b;
        fa = fb;

        b = ints[i];
        fb = fn_a(b);

        if (sgn(fa) * sgn(fb) <= 0) {
            break;
        }
    }

    alpha = fzero(a, b, fn_a, precision);

    // If alpha did not solve to anything revert the param change.
    if (std::isnan(alpha)) return false;

    m_epsilon = epsilon;
    m_alpha = alpha;

    m_Ee = -E0 * exp(alpha * Te) * sin(M_PI * Te / Tp);
    m_Te = Te;
    m_Tp = Tp;
    m_Ta = Ta;

    return true;
}

void LFModel::updateParameterBounds(GlottalFlowParameters& params) {
    params.Oq.setMin(0.35);
    params.Oq.setMax(0.9);

    params.am.setMin(0.5);
    params.am.setMax(1.0 - 1e-4);

    params.Qa.setMin(0);
    params.Qa.setMax(0.5);
}