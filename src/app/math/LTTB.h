#ifndef SOURCEMODEL__MATH_LTTB_H
#define SOURCEMODEL__MATH_LTTB_H

#include <imgui.h>
#include <implot.h>

ImPlotPoint plotGetterLTTB(int idx, void *data);

ImVector<ImPlotPoint> downsampleLTTB(const double *xs, const double *ys, int size,
                                     int downsampleSize, int start, int end);

#endif  // SOURCEMODEL__MATH_LTTB_H