#ifndef SOURCEMODEL__MATH_FREQUENCY_SCALE_H
#define SOURCEMODEL__MATH_FREQUENCY_SCALE_H

enum FrequencyScale {
    FrequencyScale_Linear = 0,
    FrequencyScale_Log2,
    FrequencyScale_Mel,
    FrequencyScale_Bark,
    FrequencyScale_ERB,
    // For bookkeeping
    FrequencyScale_COUNT,
};

const char *FrequencyScale_Name(FrequencyScale scale);

double FrequencyScale_TransformFwd(double value, void *scalePtr);
double FrequencyScale_TransformInv(double value, void *scalePtr);

#endif  // SOURCEMODEL__MATH_FREQUENCY_SCALE_H