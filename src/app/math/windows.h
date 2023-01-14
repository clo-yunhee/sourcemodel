#ifndef SOURCEMODEL__MATH_WINDOWS_H
#define SOURCEMODEL__MATH_WINDOWS_H

#include <array>
#include <boost/math/constants/constants.hpp>
#include <boost/math/special_functions/acosh.hpp>
#include <boost/math/special_functions/bessel.hpp>
#include <boost/math/special_functions/cos_pi.hpp>
#include <boost/math/special_functions/sin_pi.hpp>
#include <fftw3cxx.hh>
#include <iostream>
#include <vector>

#ifdef __EMSCRIPTEN__
    #define FFTW_SETTING FFTW_ESTIMATE
#else
    #define FFTW_SETTING FFTW_ESTIMATE
#endif

namespace windows {

using namespace boost::math::constants;
using boost::math::cos_pi;
using boost::math::sin_pi;

// Wrapper around std::vector to automatically extend & truncate for periodic windows.
template <typename T>
class vec_wrap {
   public:
    explicit vec_wrap(int& M, const bool sym)
        : _vec(!sym ? M + 1 : M, T(0)), _needsTrunc(!sym) {
        if (!sym) {
            M = M + 1;
        }
    }

    T& operator()(size_t i) { return _vec[i]; }

    operator std::vector<T>() {
        if (_needsTrunc) {
            _vec.erase(std::prev(_vec.end()), _vec.end());
        }
        return std::move(_vec);
    }

   private:
    std::vector<T> _vec;
    const bool     _needsTrunc;
};

// Linspace generator
template <typename T>
class linspace {
   public:
    explicit linspace(const T start, const T stop, const int M) : _a(M) {
        for (int i = 0; i < M; ++i) {
            _a[i] = start + (i * (stop - start)) / (M - 1);
        }
    }

    T operator()(const int i) { return _a[i]; }

   private:
    std::vector<T> _a;
};

// Sum-of-cosine templates
template <typename T, auto& coefs>
inline T cosineSum(T x) {
    constexpr size_t n(coefs.size());
    T                v(coefs[0]);
    int              s = -1;
    for (size_t i = 1; i < n; ++i) {
        v += s * coefs[i] * cos_pi(2 * i * x);
        s = -s;
    }
    return v;
}

// General sum-of-cosine window.
template <typename T, auto& coefs>
std::vector<T> generalCosine(int M, const bool sym = true) {
    vec_wrap<T> w(M, sym);
    linspace<T> fac(0, 1, M);
    for (int i = 0; i < M; ++i) {
        w(i) = cosineSum<T, coefs>(fac(i));
    }
    return w;
}

// Generalized Hamming window
template <typename T, auto& alpha>
std::vector<T> generalHamming(const int M, const bool sym = true) {
    static constexpr std::array<T, 2> a{alpha, 1.0 - alpha};
    return generalCosine<a>(M, sym);
}

// Bohman
template <typename T>
std::vector<T> bohman(int M, const bool sym = true) {
    vec_wrap<T> w(M, sym);
    linspace<T> fac(-1, 1, M);
    for (int i = 1; i < M - 1; ++i) {
        const T f = std::abs(fac(i));
        w(i) = (1 - f) * cos_pi(f) + one_div_pi<T>() * sin_pi(f);
    }
    return w;
}

// Blackman
template <typename T>
std::vector<T> blackman(const int M, const bool sym = true) {
    static constexpr std::array<T, 3> a{0.42, 0.50, 0.08};
    return generalCosine<T, a>(M, sym);
}

// Nuttall
template <typename T>
std::vector<T> nuttall(const int M, const bool sym = true) {
    static constexpr std::array<T, 4> a{0.3635819, 0.4891775, 0.1365995, 0.0106411};
    return generalCosine<T, a>(M, sym);
}

// Blackman-Harris
template <typename T>
std::vector<T> blackmanHarris(const int M, const bool sym = true) {
    static constexpr std::array<T, 4> a{0.35875, 0.48829, 0.14128, 0.01168};
    return generalCosine<T, a>(M, sym);
}

// Flat-top
template <typename T>
std::vector<T> flatTop(const int M, const bool sym = true) {
    static constexpr std::array<T, 5> a{0.21557895, 0.41663158, 0.277263158, 0.083578947,
                                        0.006947368};
    return generalCosine<T, a>(M, sym);
}

// Hann
template <typename T>
std::vector<T> hann(const int M, const bool sym = true) {
    static constexpr std::array<T, 2> a{0.5, 0.5};
    return generalCosine<T, a>(M, sym);
}

// Tukey
template <typename T>
std::vector<T> tukey(int M, const T alpha = 0.5, const bool sym = true) {
    if (alpha <= 0)
        return std::vector<T>(M, 1.0);
    else if (alpha >= 1.0)
        return hann<T>(M, sym);

    vec_wrap<T> w(M, sym);

    const int width = (int)std::floor(alpha * (M - 1) / 2.0);

    for (int i = 0; i < width + 1; ++i) {
        w(i) = 0.5 * (1 + cos_pi(-1 + 2.0 * i / alpha / (M - 1)));
    }
    for (int i = width + 1; i < M - width - 1; ++i) {
        w(i) = 1.0;
    }
    for (int i = M - width - 1; i < M; ++i) {
        w(i) = 0.5 * (1 + cos_pi(-2.0 / alpha + 1 + 2.0 * i / alpha / (M - 1)));
    }

    return w;
}

// Modified Bartlett-Hann
template <typename T>
std::vector<T> barthann(int M, const bool sym = true) {
    vec_wrap<T> w(M, sym);
    linspace<T> fac(-0.5, 0.5, M);
    for (int i = 0; i < M; ++i) {
        const T f = std::abs(fac(i));
        w(i) = 0.62 - 0.48 * f + 0.38 * cos_pi(2 * f);
    }
    return w;
}

// Hamming
template <typename T>
std::vector<T> hamming(const int M, const bool sym = true) {
    static constexpr std::array<T, 2> a{0.54, 0.46};
    return generalCosine<T, a>(M, sym);
}

// Kaiser
template <typename T>
std::vector<T> kaiser(int M, const T beta, const bool sym = true) {
    vec_wrap<T> w(M, sym);
    const T     alpha = (M - 1) / 2.0;
    const T     i0_beta = boost::math::cyl_bessel_i(0, beta);
    for (int i = 0; i < M; ++i) {
        const T iaa = (i - alpha) / alpha;
        const T i0_x = boost::math::cyl_bessel_i(0, beta * std::sqrt(1 - iaa * iaa));
        w(i) = i0_x / i0_beta;
    }
    return w;
}

// Kaiser-Bessel derived
template <typename T>
std::vector<T> kaiserBesselDerived(const int M, const T beta, const bool sym = true) {
    if (!sym) {
        throw std::invalid_argument(
            "Kaiser-Bessel derived windows are only defined for symmetric shapes");
    } else if (M < 1) {
        return {};
    } else if (M % 2 == 1) {
        throw std::invalid_argument(
            "Kaiser-Bessel derived windows are only defined for even number of points");
    }

    std::vector<T> kaiserWindow(M / 2 + 1, beta);
    std::vector<T> csum(M / 2 + 1);
    std::vector<T> w(M);

    csum[0] = kaiserWindow[0];

    // Kahan summation. (M might be large enough to accumulate a big error otherwise)
    T c(0);
    for (int i = 1; i < M / 2 + 1; ++i) {
        // w(i) = w(i-1) + kaiser(i)
        const T          y(kaiserWindow[i] - c);
        volatile const T t(csum[i - 1] + y);
        volatile const T z(t - csum[i - 1]);
        c = z - y;
        csum[i] = t;
    }

    for (int i = 0; i < M / 2; ++i) {
        w[i] = w[M - 1 - i] = std::sqrt(csum[i] / csum[M / 2 + 1]);
    }

    return w;
}

// Gaussian
template <typename T>
std::vector<T> gaussian(int M, const T std, const bool sym = true) {
    vec_wrap<T> w(M, sym);
    const T     sig2 = 2 * std * std;
    for (int i = 0; i < M; ++i) {
        const T n = i - (M - 1) / 2.0;
        w(i) = std::exp(-(n * n) / sig2);
    }
    return w;
}

// Dolph-Chebyshev
template <typename T>
std::vector<T> chebwin(int M, const T at, const bool sym = true) {
    if (std::abs(at) < 45) {
        std::cerr << "This window is not suitable for spectral analysis "
                     "for attenuation values lower than about 45dB because "
                     "the equivalent noise bandwidth of a Chebyshev window "
                     "does not grow monotonically with increasing sidelobe "
                     "attenuation when the attenuation is smaller than "
                     "about 45 dB."
                  << std::endl;
    }
    using boost::math::acosh;

    vec_wrap<T> w(M, sym);

    // Compute the parameter beta
    const T order = M - 1;
    const T beta = std::cosh(T(1) / order * acosh(std::pow(T(10), std::abs(at) / T(20))));
    std::vector<T> x(M);
    for (int i = 0; i < M; ++i) {
        x[i] = beta * cos_pi(T(i) / T(M));
    }

    std::vector<std::complex<T>> p(M);

    auto plan =
        fftw3cxx::plan<T>::plan_dft_1d(M, p.data(), p.data(), FFTW_FORWARD, FFTW_SETTING);

    // Find the window's DFT coefficients
    // Use analytic definition of Chebyshev polynomial instead of expansion
    for (int i = 0; i < M; ++i) {
        if (x[i] > 1) {
            p[i] = std::cosh(order * acosh(x[i]));
        } else if (x[i] < -1) {
            p[i] = (2 * (M % 2) - 1) * std::cosh(order * acosh(-x[i]));
        } else {
            p[i] = std::cos(order * std::acos(x[i]));
        }
    }

    // Appropriate IDFT and filling up depending on even/odd M.
    if (M % 2 != 0) {
        plan.execute();

        const int n = (M + 1) / 2;
        for (int i = 0; i < n; ++i) {
            const T pi = std::real(p[i]);
            if (i > 0) w(n - 1 - i) = pi;
            w(n - 1 + i) = pi;
        }
    } else {
        for (int i = 0; i < M; ++i) {
            p[i] = std::polar(std::real(p[i]), pi<T>() / M * i);
        }

        plan.execute();

        const int n = M / 2 + 1;
        for (int i = 1; i < n; ++i) {
            const T pi = std::real(p[i]);
            w(n - 1 - i) = pi;
            w(n - 2 + i) = pi;
        }
    }

    // Scale.
    T maxW = std::numeric_limits<T>::min();
    for (int i = 0; i < M; ++i) {
        if (w(i) > maxW) {
            maxW = w(i);
        }
    }
    for (int i = 0; i < M; ++i) {
        w(i) /= maxW;
    }

    return w;
}

}  // namespace windows

#endif  // SOURCEMODEL__MATH_WINDOWS_H