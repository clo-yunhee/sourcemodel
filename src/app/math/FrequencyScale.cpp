#include "FrequencyScale.h"

#include <cmath>
#include <limits>

using TransformFn = double (*)(double);

/* Linear scale */

static inline double TransformLinear(double v) { return v; }

/* Log2 scale */

static inline double TransformLog2_Forward(double v) {
    v = v <= 0.0 ? std::numeric_limits<double>::lowest() : v;
    return log2(v);
}

static inline double TransformLog2_Inverse(double v) { return pow(2, v); }

/* Mel scale */

static inline double TransformMel_Forward(double f) { return 2595 * log10(1 + f / 700); }

static inline double TransformMel_Inverse(double m) {
    return 700 * (pow(10, m / 2595) - 1);
}

/* Bark scale */

static inline double TransformBark_Forward(double f) {
    double b = (26.81 * f) / (1960 + f) - 0.53;
    if (b < 2) b = b + 0.15 * (2 - b);
    if (b > 20.1) b = b + 0.22 * (b - 20.1);
    return b;
}

static inline double TransformBark_Inverse(double b) {
    if (b < 2) b = (b - 0.3) / 0.85;
    if (b > 20.1) b = (b + 4.422) / 1.22;
    return 1960 * (b + 0.53) / (26.28 - b);
}

/* ERB scale */

static inline double TransformErb_Forward(double f) {
    return 21.33228 * log10(1 + 0.00437 * f);
}

static inline double TransformErb_Inverse(double erb) {
    return (pow(10, erb / 21.33228) - 1) / 0.00437;
}

/* Data arrays */

static constexpr const char *g_ScaleNames[FrequencyScale_COUNT] = {
    "Linear", "Logarithmic", "Mel", "Bark", "ERB",
};

static constexpr TransformFn g_ScaleTransformFwd[FrequencyScale_COUNT] = {
    TransformLinear,       TransformLog2_Forward, TransformMel_Forward,
    TransformBark_Forward, TransformErb_Forward,
};

static constexpr TransformFn g_ScaleTransformInv[FrequencyScale_COUNT] = {
    TransformLinear,       TransformLog2_Inverse, TransformMel_Inverse,
    TransformBark_Inverse, TransformErb_Inverse,
};

/* Public interface functions */

const char *FrequencyScale_Name(FrequencyScale scale) { return g_ScaleNames[scale]; }

double FrequencyScale_TransformFwd(double value, void *scalePtr) {
    const auto index = *(FrequencyScale *)scalePtr;
    return g_ScaleTransformFwd[index](value);
}

double FrequencyScale_TransformInv(double value, void *scalePtr) {
    const auto index = *(FrequencyScale *)scalePtr;
    return g_ScaleTransformInv[index](value);
}