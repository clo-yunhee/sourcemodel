#include "LF.h"

#include <array>
#include <boost/math/constants/constants.hpp>
#include <boost/math/quadrature/gauss_kronrod.hpp>
#include <boost/math/special_functions/cos_pi.hpp>
#include <boost/math/special_functions/sin_pi.hpp>
#include <boost/math/tools/roots.hpp>
#include <iostream>

namespace math = boost::math;
using namespace boost::math::constants;
using namespace boost::math::quadrature;
using namespace models;
using namespace models::precomp;
using boost::math::cos_pi;
using boost::math::sin_pi;

Scalar LF::evaluate(Scalar t) const {
    static constexpr Scalar T0 = 1;

    Scalar dg;

    if (t < 0 || t > 1) {
        t = -std::fmod(std::abs(t), 1.0_f);
    }

    if (t <= m_Te) {
        dg = -m_Ee * std::exp(m_alpha * (t - m_Te)) * sin_pi(t / m_Tp) /
             sin_pi(m_Te / m_Tp);
    } else if (!std::isinf(m_epsilon) && !std::isnan(m_epsilon)) {
        dg = -m_Ee / (m_epsilon * m_Ta) *
             (std::exp(-m_epsilon * (t - m_Te)) - std::exp(-m_epsilon * (T0 - m_Te)));
    } else {
        dg = 0;
    }
    return dg;
}

Scalar LF::evaluateAntiderivative(Scalar t) const {
    static constexpr Scalar T0 = 1;

    Scalar g;

    if (t < 0 || t > 1) {
        t = -std::fmod(std::abs(t), 1.0_f);
    }

    if (t <= m_Te) {
        g = -(m_Ee * std::exp(-m_alpha * m_Te)) / sin_pi(m_Te / m_Tp) * 1.0_f /
            (m_alpha * m_alpha + pi_sqr<Scalar>() / (m_Tp * m_Tp)) *
            (pi<Scalar>() / m_Tp + m_alpha * std::exp(m_alpha * t) * sin_pi(t / m_Tp) -
             pi<Scalar>() / m_Tp * std::exp(m_alpha * t) * cos_pi(t / m_Tp));
    } else {
        // Start integrating from Te.
        // Doing it this way means we are integrating a smooth function.
        const auto& fn = [this](Scalar t) -> Scalar { return evaluate(t); };

        // Float / double
#if defined(USING_SINGLE_FLOAT)
        g = m_gTe + gauss_kronrod<float, 15>::integrate(fn, m_Te, t, 3, 1e-3);
#elif defined(USING_DOUBLE_FLOAT)
        g = m_gTe + gauss_kronrod<double, 31>::integrate(fn, m_Te, t, 3, 1e-3);
#endif
    }
    return g;
}

void LF::fitParameters(const GlottalFlowParameters& params) {
    static constexpr Scalar Ee = 1;
    static constexpr Scalar T0 = 1;

    // Check if using Rd or not.

    if (!params.usingRd()) {
        const Scalar Oq = params.Oq.value();
        const Scalar am = params.am.value();
        const Scalar Qa = params.Qa.value();

        const Scalar Ta = Qa * (1 - Oq) * T0;
        const Scalar Te = Oq * T0;
        const Scalar Tp = (am > 0.5 ? am : 0.5 + 1e-6) * Oq * T0;

        fitParameters(Ee, T0, Te, Tp, Ta);
    } else {
        // Just interpolate the pre-computed values.
        const Scalar Rd = params.Rd.value();
        const int    index = std::floor((Rd - Rd_min) / Rd_step);

        m_Ee = Ee;
        std::tie(m_Te, m_Tp, m_Ta, m_alpha, m_epsilon) = Rd_table[index];
    }

    // Store Ug(Te)
    m_gTe = evaluateAntiderivative(m_Te);
}

void LF::updateParameterBounds(GlottalFlowParameters& params) {
    params.Oq.setMin(0.35_f);
    params.Oq.setMax(0.85_f);

    params.am.setMin(0.55_f);
    params.am.setMax(0.915_f);

    params.Qa.setMin(0_f);
    params.Qa.setMax(0.9_f);
}

Scalar LF::Te() const { return m_Te; }

void LF::fitParameters(const Scalar Ee, const Scalar T0, const Scalar Te, const Scalar Tp,
                       const Scalar Ta) {
    Scalar epsilon;
    Scalar alpha;

    // If Ta is zero we can skip this and just assign a big value to epsilon.
    if (Ta > 0) {
        // epsilon is expressed by an implicit equation
        const auto fn_e = [=](const Scalar e) {
            const Scalar expComp = std::exp(-e * (T0 - Te));
            const Scalar f = 1 - expComp - e * Ta;
            const Scalar df = (1 - Te) * expComp - Ta;
            const Scalar d2f = -(1 - Te) * (1 - Te) * expComp;

            return std::make_tuple(f, df, d2f);
        };

        uintmax_t maxit = 1'000;
        epsilon = math::tools::schroder_iterate(
            fn_e, 1_f / Ta, 0.0_f, 2_f / Ta, std::numeric_limits<Scalar>::digits, maxit);

        // If epsilon did not solve to anything revert the param change.
        if (std::isnan(epsilon)) return;
    } else {
        epsilon = std::numeric_limits<Scalar>::infinity();
    }

    // alpha is expressed by another implicit equation
    const Scalar wg = pi<Scalar>() / Tp;
    const Scalar wg2 = pi_sqr<Scalar>() / (Tp * Tp);
    const Scalar sinwgTe = sin_pi(Te / Tp);
    const Scalar coswgTe = cos_pi(Te / Tp);
    const Scalar wgsinwgTe = wg / sinwgTe;

    Scalar A;

    // If Ta is small enough we can remove epsilon from this.
    if (!std::isinf(epsilon)) {
        A = (T0 - Te) / (std::exp(epsilon * (T0 - Te)) - 1) - 1 / epsilon;
    } else {
        A = 0;
    }

    const auto fn_a = [=](const Scalar a) {
        return 1 / (a * a + wg2) * ((std::exp(-a * Te) - coswgTe) * wgsinwgTe + a) - A;
    };

    // Find the first interval with a zero crossing
    std::array ints{-1e20, -1e9, -1e8, -1e7, -1e6, -1e5, -1e4, -1e3, -1e2, -1e1, 0.0,
                    1e1,   1e2,  1e3,  1e4,  1e5,  1e6,  1e7,  1e8,  1e9,  1e20};
    Scalar     fa, fb;
    Scalar     a, b;

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

    std::uintmax_t maxit = 1'000;
    Scalar         alpha2;
    std::tie(alpha, alpha2) = math::tools::toms748_solve(
        fn_a, a, b, math::tools::eps_tolerance<Scalar>(), maxit);

    // If alpha did not solve to anything revert the param change.
    if (std::isnan(alpha)) return;

    m_epsilon = epsilon;
    m_alpha = alpha;

    m_Ee = Ee;
    m_Te = Te;
    m_Tp = Tp;
    m_Ta = Ta;
}