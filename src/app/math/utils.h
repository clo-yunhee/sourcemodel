#ifndef SOURCEMODEL__MATH_UTILS_H
#define SOURCEMODEL__MATH_UTILS_H

#include <cmath>
#include <limits>

#if defined(USING_SINGLE_FLOAT)
using Scalar = float;
#elif defined(USING_DOUBLE_FLOAT)
using Scalar = double;
#endif

inline constexpr Scalar operator""_f(const long double x) { return Scalar(x); }
inline constexpr Scalar operator""_f(unsigned long long int x) { return Scalar(x); }

template <typename T>
inline bool fuzzyEquals(const T a, const T b,
                        const T epsilon = std::numeric_limits<T>::epsilon()) {
    return std::abs(a - b) <=
           (std::abs(a) < std::abs(b) ? std::abs(b) : std::abs(a)) * epsilon;
}

template <typename T>
inline int nextpow2(const T a) {
    if constexpr (std::is_integral_v<T>) {
        int i;
        for (i = -1; a; ++i, a >>= 1)
            ;
        return i;
    } else {
        return (int)std::ceil(std::log2(a));
    }
}

#endif  // SOURCEMODEL__MATH_UTILS_H