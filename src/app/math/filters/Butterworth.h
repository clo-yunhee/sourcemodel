#ifndef SOURCEMODEL__MATH_FILTERS_BUTTERWORTH_H
#define SOURCEMODEL__MATH_FILTERS_BUTTERWORTH_H

#include "SOSFilter.h"

namespace butterworth {
SOSFilter lowPass(int N, double fc, double fs);
SOSFilter highPass(int N, double fc, double fs);
}  // namespace butterworth

#endif  // SOURCEMODEL__MATH_FILTERS_BUTTERWORTH_H