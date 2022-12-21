#include "LF.h"

#include <array>
#include <boost/math/constants/constants.hpp>
#include <boost/math/special_functions/cos_pi.hpp>
#include <boost/math/special_functions/sin_pi.hpp>
#include <boost/math/tools/roots.hpp>
#include <iostream>

namespace math = boost::math;
using namespace boost::math::double_constants;
using namespace models;
using boost::math::cos_pi;
using boost::math::sin_pi;

double LF::evaluate(double t) const {
    static constexpr double T0 = 1;

    double dg;

    if (t < 0 || t > 1) {
        t = -fmod(fabs(t), 1.0);
    }

    if (t <= m_Te) {
        dg = -m_Ee * std::exp(m_alpha * (t - m_Te)) * sin_pi(t / m_Tp) /
             sin_pi(m_Te / m_Tp);
    } else if (!std::isinf(m_epsilon)) {
        dg = -m_Ee / (m_epsilon * m_Ta) *
             (std::exp(-m_epsilon * (t - m_Te)) - std::exp(-m_epsilon * (T0 - m_Te)));
    } else {
        dg = 0;
    }
    return dg;
}

void LF::fitParameters(const GlottalFlowParameters& params) {
    static constexpr double Ee = 1;
    static constexpr double T0 = 1;

    // Check if using Rd or not.

    if (!params.usingRd()) {
        const double Oq = params.Oq.value();
        const double am = params.am.value();
        const double Qa = params.Qa.value();

        const double Ta = Qa * (1 - Oq) * T0;
        const double Te = Oq * T0;
        const double Tp = (am > 0.5 ? am : 0.5 + 1e-6) * Oq * T0;

        fitParameters(Ee, T0, Te, Tp, Ta);
    } else {
        const double Rd = params.Rd.value();

        double Rap;
        double Rkp;
        double Rgp;

        if (0.01 <= Rd && Rd < 0.21) {
            Rap = 0;
        } else if (0.21 <= Rd && Rd <= 2.70) {
            Rap = (-1 + 4.8 * Rd) / 100;
        } else if (2.70 < Rd && Rd <= 6.00) {
            Rap = (32.3 / Rd) / 100;
        }

        double OQupp;
        if (1.8476 <= Rd && Rd <= 6.00) {
            OQupp = 1 - 1 / (2.17 * Rd);
        }

        constexpr bool variant1 = false;

        if constexpr (variant1) {
            // Variant 1.

            if (1.8476 < Rd && Rd <= 6.00) {
                Rgp = 9.3552e-3 + 596e-2 / (7.96 - 2 * OQupp);
            }

            if (0.01 <= Rd && Rd <= 2.70) {
                Rkp = (22.4 + 11.8 * Rd) / 100;
            } else if (2.70 < Rd && Rd <= 6.00) {
                Rkp = (2 * Rgp * OQupp) - 1.0428;
            }

            if (0.01 <= Rd && Rd <= 1.8476) {
                Rgp = (0.25 * Rkp) / ((0.11 * Rd) / (0.5 + 1.2 * Rkp) - Rap);
            }
        } else {
            // Variant 2.

            if (1.8476 < Rd && Rd <= 6.00) {
                Rgp = 9.3552e-3 + 596e-2 / (7.96 - 2 * OQupp);
            }

            if (0.01 <= Rd && Rd <= 1.8476) {
                Rkp = (22.4 + 11.8 * Rd) / 100;
            } else if (1.8476 < Rd && Rd <= 6.00) {
                Rkp = (2 * Rgp * OQupp) - 0.9572;
            }

            if (0.01 <= Rd && Rd <= 1.8476) {
                Rgp = (0.25 * Rkp) / ((0.11 * Rd) / (0.5 + 1.2 * Rkp) - Rap);
            }
        }

        // Ra = Ta / T0
        // Rg = T0 / 2Tp
        // Rk = (Te - Tp) / Tp

        // Ta = Ra T0
        // Tp = T0 / 2Rg
        // Te = T0 (1 + Rk) / 2Rg

        const double Ta = Rap * T0;
        const double Tp = T0 / (2 * Rgp);
        const double Te = T0 * (1 + Rkp) / (2 * Rgp);

        fitParameters(Ee, T0, Te, Tp, Ta);
    }
}

void LF::updateParameterBounds(GlottalFlowParameters& params) {
    params.Oq.setMin(0.35);
    params.Oq.setMax(0.85);

    params.am.setMin(0.55);
    params.am.setMax(0.915);

    params.Qa.setMin(0);
    params.Qa.setMax(0.9);
}

double LF::Te() const { return m_Te; }

void LF::fitParameters(const double Ee, const double T0, const double Te, const double Tp,
                       const double Ta) {
    static constexpr double precision = std::numeric_limits<double>::epsilon();

    double epsilon;
    double alpha;

    // If Ta is small enough we can skip this and just assign a big value to epsilon.
    if (Ta > precision) {
        // epsilon is expressed by an implicit equation
        const auto fn_e = [=](const double e) {
            const double expComp = std::exp(-e * (T0 - Te));
            const double f = 1 - expComp - e * Ta;
            const double df = (1 - Te) * expComp - Ta;
            const double d2f = -(1 - Te) * (1 - Te) * expComp;

            return std::make_tuple(f, df, d2f);
        };

        uintmax_t maxit = 1000;
        epsilon = math::tools::schroder_iterate(
            fn_e, 1 / Ta, 0.0, 2 / Ta, std::numeric_limits<double>::digits, maxit);

        // If epsilon did not solve to anything revert the param change.
        if (std::isnan(epsilon)) return;
    } else {
        epsilon = std::numeric_limits<double>::infinity();
    }

    // alpha is expressed by another implicit equation
    const double wg = pi / Tp;
    const double wg2 = pi_sqr / (Tp * Tp);
    const double sinwgTe = sin_pi(Te / Tp);
    const double coswgTe = cos_pi(Te / Tp);
    const double wgsinwgTe = wg / sinwgTe;

    double A;

    // If Ta is small enough we can remove epsilon from this.
    if (!std::isinf(epsilon)) {
        A = (T0 - Te) / (std::exp(epsilon * (T0 - Te)) - 1) - 1 / epsilon;
    } else {
        A = 0;
    }

    const auto fn_a = [=](const double a) {
        return 1 / (a * a + wg2) * ((std::exp(-a * Te) - coswgTe) * wgsinwgTe + a) - A;
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
        if (std::signbit(fa) != std::signbit(fb)) break;
    }

    b = ints[0];
    fb = fn_a(b);
    for (int i = 1; i < ints.size(); ++i) {
        a = b;
        fa = fb;
        b = ints[i];
        fb = fn_a(b);
        if (std::signbit(fa) != std::signbit(fb)) break;
    }

    std::uintmax_t maxit = 10000;
    double         alpha2;
    std::tie(alpha, alpha2) = math::tools::toms748_solve(
        fn_a, a, b, math::tools::eps_tolerance<double>(), maxit);

    // If alpha did not solve to anything revert the param change.
    if (std::isnan(alpha)) return;

    m_epsilon = epsilon;
    m_alpha = alpha;

    m_Ee = Ee;
    m_Te = Te;
    m_Tp = Tp;
    m_Ta = Ta;
}