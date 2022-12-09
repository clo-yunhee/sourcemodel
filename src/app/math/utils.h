#ifndef SOURCEMODEL__MATH_UTILS_H
#define SOURCEMODEL__MATH_UTILS_H

#include <cmath>
#include <limits>

inline bool fuzzyEquals(double a, double b,
                        double epsilon = std::numeric_limits<double>::epsilon()) {
    return fabs(a - b) <= (fabs(a) < fabs(b) ? fabs(b) : fabs(a)) * epsilon;
}

#endif  // SOURCEMODEL__MATH_UTILS_H