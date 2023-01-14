#include <boost/math/special_functions/cos_pi.hpp>
#include <boost/math/special_functions/sign.hpp>
#include <boost/math/special_functions/sin_pi.hpp>
#include <boost/math/tools/roots.hpp>
#include <boost/multiprecision/cpp_bin_float.hpp>
#include <boost/multiprecision/cpp_dec_float.hpp>
#include <fstream>
#include <iostream>

namespace math = boost::math;
namespace mpr = boost::multiprecision;

using Scalar = mpr::cpp_bin_float_oct;

struct Params {
    Scalar Rd;
    Scalar T0, Te, Tp, Ta;
    Scalar alpha;
    Scalar epsilon;
};

struct DblParams {
    double Rd;
    double Te, Tp, Ta, alpha, epsilon;
};

static const Scalar pi = math::constants::pi<Scalar>();
static const Scalar pi_sqr = math::constants::pi_sqr<Scalar>();

static void calculateBasicParameters(Params& p) {
    const Scalar s600 = Scalar("7.0");  // Actually 6.00.

    const Scalar Rd = p.Rd;

    Scalar Rap;
    Scalar Rkp;
    Scalar Rgp;

    if (Scalar("0.01") <= Rd && Rd < Scalar("0.21")) {
        Rap = 0;
    } else if (Scalar("0.21") <= Rd && Rd <= Scalar("2.70")) {
        Rap = (-1 + Scalar("4.8") * Rd) / 100;
    } else if (Scalar("2.70") < Rd && Rd <= s600) {
        Rap = (Scalar("32.3") / Rd) / 100;
    }

    Scalar OQupp;
    if (Scalar("1.8476") <= Rd && Rd <= s600) {
        OQupp = 1 - 1 / (Scalar("2.17") * Rd);
    }

    constexpr bool variant1 = false;

    if constexpr (variant1) {
        // Variant 1.

        if (Scalar("1.8476") < Rd && Rd <= s600) {
            Rgp = Scalar("9.3552e-3") + Scalar("596e-2") / (Scalar("7.96") - 2 * OQupp);
        }

        if (Scalar("0.01") <= Rd && Rd <= Scalar("2.70")) {
            Rkp = (Scalar("22.4") + Scalar("11.8") * Rd) / 100;
        } else if (Scalar("2.70") < Rd && Rd <= s600) {
            Rkp = (2 * Rgp * OQupp) - Scalar("1.0428");
        }

        if (Scalar("0.01") <= Rd && Rd <= Scalar("1.8476")) {
            Rgp = (Scalar("0.25") * Rkp) /
                  ((Scalar("0.11") * Rd) / (Scalar("0.5") + Scalar("1.2") * Rkp) - Rap);
        }
    } else {
        // Variant 2.

        if (Scalar("1.8476") < Rd && Rd <= s600) {
            Rgp = Scalar("9.3552e-3") + Scalar("596e-2") / (Scalar("7.96") - 2 * OQupp);
        }

        if (Scalar("0.01") <= Rd && Rd <= Scalar("1.8476")) {
            Rkp = (Scalar("22.4") + Scalar("11.8") * Rd) / 100;
        } else if (Scalar("1.8476") < Rd && Rd <= s600) {
            Rkp = (2 * Rgp * OQupp) - Scalar("0.9572");
        }

        if (Scalar("0.01") <= Rd && Rd <= Scalar("1.8476")) {
            Rgp = (Scalar("0.25") * Rkp) /
                  ((Scalar("0.11") * Rd) / (Scalar("0.5") + Scalar("1.2") * Rkp) - Rap);
        }
    }

    // Ra = Ta / T0
    // Rg = T0 / 2Tp
    // Rk = (Te - Tp) / Tp

    // Ta = Ra T0
    // Tp = T0 / 2Rg
    // Te = T0 (1 + Rk) / 2Rg

    p.Ta = Rap * p.T0;
    p.Tp = p.T0 / (2 * Rgp);
    p.Te = p.T0 * (1 + Rkp) / (2 * Rgp);
}

static void calculateEpsilon(Params& p) {
    const Scalar T0 = p.T0;
    const Scalar Te = p.Te;
    const Scalar Ta = p.Ta;

    if (Ta > 0) {
        const auto fn_e = [=](const Scalar e) {
            const Scalar expComp = mpr::exp(-e * (T0 - Te));
            const Scalar f = 1 - expComp - e * Ta;
            const Scalar df = (1 - Te) * expComp - Ta;
            const Scalar d2f = -(1 - Te) * (1 - Te) * expComp;

            return std::make_tuple(f, df, d2f);
        };

        uintmax_t maxit = 1'000'000;
        p.epsilon = math::tools::schroder_iterate(
            fn_e, 1 / Ta, Scalar(0), 2 / Ta, std::numeric_limits<Scalar>::digits, maxit);
    } else {
        p.epsilon = std::numeric_limits<Scalar>::infinity();
    }
}

static void calculateAlpha(Params& p) {
    const Scalar T0 = p.T0;
    const Scalar Te = p.Te;
    const Scalar Tp = p.Tp;
    const Scalar epsilon = p.epsilon;

    const Scalar wg = pi / Tp;
    const Scalar wg2 = pi_sqr / (Tp * Tp);
    const Scalar sinwgTe = math::sin_pi(Te / Tp);
    const Scalar coswgTe = math::cos_pi(Te / Tp);
    const Scalar wgsinwgTe = wg / sinwgTe;

    Scalar A;
    if (!std::isinf(epsilon)) {
        A = (T0 - Te) / (mpr::exp(epsilon * (T0 - Te)) - 1) - 1 / epsilon;
    } else {
        A = 0;
    }

    const auto fn_a = [=](const Scalar a) {
        return 1 / (a * a + wg2) * ((mpr::exp(-a * Te) - coswgTe) * wgsinwgTe + a) - A;
    };

    // Find the first interval with a zero crossing.
    const int           range = 80;
    std::vector<Scalar> ints;
    for (int exp = range; exp >= 1; --exp) {
        ints.push_back(Scalar("-1e" + std::to_string(exp)));
    }
    ints.push_back(Scalar(0));
    for (int exp = 1; exp <= range; ++exp) {
        ints.push_back(Scalar("1e" + std::to_string(exp)));
    }

    Scalar a, fa, b, fb;

    for (int i = 0; i < ints.size() - 1; ++i) {
        a = ints[i];
        fa = fn_a(a);
        b = ints[i + 1];
        fb = fn_a(b);
        if (math::signbit(fa) != math::signbit(fb)) break;
    }

    b = ints[0];
    fb = fn_a(b);
    for (int i = 1; i < ints.size(); ++i) {
        a = b;
        fa = fb;
        b = ints[i];
        fb = fn_a(b);
        if (math::signbit(fa) != math::signbit(fb)) break;
    }

    uintmax_t maxit = 1'000'000;
    Scalar    alpha2;
    std::tie(p.alpha, alpha2) = math::tools::toms748_solve(
        fn_a, a, b, math::tools::eps_tolerance<Scalar>(), maxit);
}

static inline double oct2double(const Scalar x) { return x.convert_to<double>(); }

struct _ff {
    _ff(float v) : _v(v) {}
    float _v;
};

std::ostream& operator<<(std::ostream& os, const _ff& f) {
    if (f._v == (int)f._v) {
        return os << f._v << ".0f";
    } else {
        return os << f._v << "f";
    }
}

int main(int argc, char** argv) {
    Params p;
    p.T0 = 1;

    const Scalar Rdmin("0.01");
    const Scalar Rdmax("6.000");
    const Scalar deltaRd("0.001");

    std::vector<DblParams> results;
    DblParams              r;

    std::cout << std::setprecision(std::numeric_limits<Scalar>::max_digits10);

    std::cout << "Precomputing LF parameters from Rd values.\n";
    std::cout << "Rdmin = " << Rdmin << ", Rdmax = " << Rdmax << "\n";

    const int numRd = 1 + (int)mpr::floor((Rdmax - Rdmin) / deltaRd);

    for (int i = 0; i <= numRd; ++i) {
        const Scalar Rd(Rdmin + i * deltaRd);

        std::cout << " -- Rd = " << Rd << std::flush;

        p.Rd = Rd;
        calculateBasicParameters(p);
        calculateEpsilon(p);
        calculateAlpha(p);

        r.Rd = oct2double(p.Rd);
        r.Te = oct2double(p.Te);
        r.Tp = oct2double(p.Tp);
        r.Ta = oct2double(p.Ta);
        r.alpha = oct2double(p.alpha);
        r.epsilon = oct2double(p.epsilon);
        results.push_back(r);

        std::cout << std::endl;
    }

    std::cout << "Finished precomputing, priting to file.\n";

    std::ofstream file("LF_precomputed_Rd_double.inc.h", std::ios_base::trunc);

    file << std::setprecision(std::numeric_limits<double>::max_digits10);

    file << "inline constexpr double Rd_min = " << Rdmin << ";\n"
         << "inline constexpr double Rd_max = " << Rdmax << ";\n"
         << "inline constexpr double Rd_step = " << deltaRd << ";\n"
         << "\n"
         << "// (Te, Tp, Ta, alpha, epsilon)\n"
         << "inline constexpr std::array<std::tuple<double, double, double, double, "
            "double>, "
         << results.size() << ">\n    Rd_table{{\n";

    for (const auto& r : results) {
        file << "        {\n"
             << "            // Rd = " << r.Rd << "\n"
             << "            " << r.Te << ",\n"
             << "            " << r.Tp << ",\n"
             << "            " << r.Ta << ",\n"
             << "            " << r.alpha << ",\n";
        if (std::isinf(r.epsilon)) {
            file << "            std::numeric_limits<double>::infinity(),\n";
        } else {
            file << "            " << r.epsilon << ",\n";
        }
        file << "        },\n";
    }

    file << "    }};\n";
    file.close();

    file.open("LF_precomputed_Rd_float.inc.h", std::ios_base::trunc);

    file << std::setprecision(std::numeric_limits<float>::max_digits10);

    file << "inline constexpr float Rd_min = " << Rdmin << ";\n"
         << "inline constexpr float Rd_max = " << Rdmax << ";\n"
         << "inline constexpr float Rd_step = " << deltaRd << ";\n"
         << "\n"
         << "// (Te, Tp, Ta, alpha, epsilon)\n"
         << "inline constexpr std::array<std::tuple<float, float, float, float, "
            "float>, "
         << results.size() << ">\n    Rd_table{{\n";

    for (const auto& r : results) {
        file << "        {\n"
             << "            // Rd = " << r.Rd << "\n"
             << "            " << _ff(r.Te) << ",\n"
             << "            " << _ff(r.Tp) << ",\n"
             << "            " << _ff(r.Ta) << ",\n"
             << "            " << _ff(r.alpha) << ",\n";
        if (std::isinf(r.epsilon)) {
            file << "            std::numeric_limits<float>::infinity(),\n";
        } else {
            file << "            " << _ff(r.epsilon) << ",\n";
        }
        file << "        },\n";
    }

    file << "    }};\n";
    file.close();

    return 0;
}