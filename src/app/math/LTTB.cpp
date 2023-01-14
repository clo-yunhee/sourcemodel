#include "LTTB.h"

#include <cfloat>
#include <cmath>

inline ImPlotPoint GetDataAt(const Scalar *xs, const Scalar *ys, int offset, int idx) {
    return ImPlotPoint(xs[offset + idx], ys[offset + idx]);
}

ImPlotPoint plotGetterLTTB(int idx, void *data) {
    ImPlotPoint *ds = (ImPlotPoint *)data;
    return ds[idx];
}

ImVector<ImPlotPoint> downsampleLTTB(const Scalar *xs, const Scalar *ys, int size,
                                     int downsampleSize, int start, int end) {
    // Largest Triangle Three Buckets (LTTB) Downsampling Algorithm
    //  "Downsampling time series for visual representation" by Sveinn Steinarsson.
    //  https://skemman.is/bitstream/1946/15343/3/SS_MSthesis.pdf
    //  https://github.com/sveinn-steinarsson/flot-downsample
    ImVector<ImPlotPoint> Ds;

    int rawSamplesCount = (end - start) + 1;
    if (rawSamplesCount > downsampleSize) {
        int    sampleIdxOffset = start;
        Scalar every = ((Scalar)rawSamplesCount) / ((Scalar)downsampleSize);
        int    aIndex = 0;
        // fill first sample
        Ds.resize(downsampleSize);
        Ds[0] = GetDataAt(xs, ys, sampleIdxOffset, 0);
        // loop over samples
        for (int i = 0; i < downsampleSize - 2; ++i) {
            int avgRangeStart = (int)(i * every) + 1;
            int avgRangeEnd = (int)((i + 1) * every) + 1;
            if (avgRangeEnd > downsampleSize) avgRangeEnd = downsampleSize;

            int    avgRangeLength = avgRangeEnd - avgRangeStart;
            Scalar avgX = 0.0_f;
            Scalar avgY = 0.0_f;
            for (; avgRangeStart < avgRangeEnd; ++avgRangeStart) {
                ImPlotPoint sample = GetDataAt(xs, ys, sampleIdxOffset, avgRangeStart);
                if (sample.y != NAN) {
                    avgX += sample.x;
                    avgY += sample.y;
                }
            }
            avgX /= (Scalar)avgRangeLength;
            avgY /= (Scalar)avgRangeLength;

            int rangeOffs = (int)(i * every) + 1;
            int rangeTo = (int)((i + 1) * every) + 1;
            if (rangeTo > downsampleSize) rangeTo = downsampleSize;
            ImPlotPoint samplePrev = GetDataAt(xs, ys, sampleIdxOffset, aIndex);
            Scalar      maxArea = -1.0_f;
            int         nextAIndex = rangeOffs;
            for (; rangeOffs < rangeTo; ++rangeOffs) {
                ImPlotPoint sampleAtRangeOffs =
                    GetDataAt(xs, ys, sampleIdxOffset, rangeOffs);
                if (sampleAtRangeOffs.y != NAN) {
                    Scalar area = std::abs((samplePrev.x - avgX) *
                                               (sampleAtRangeOffs.y - samplePrev.y) -
                                           (samplePrev.x - sampleAtRangeOffs.x) *
                                               (avgY - samplePrev.y)) /
                                  2.0_f;
                    if (area > maxArea) {
                        maxArea = area;
                        nextAIndex = rangeOffs;
                    }
                }
            }
            Ds[i + 1] = GetDataAt(xs, ys, sampleIdxOffset, nextAIndex);
            aIndex = nextAIndex;
        }
        // fill last sample
        Ds[downsampleSize - 1] = GetDataAt(xs, ys, sampleIdxOffset, rawSamplesCount - 1);
    } else {
        int sampleIdxOffset = start;
        // loop over samples
        Ds.resize(rawSamplesCount);
        for (int i = 0; i < rawSamplesCount; ++i) {
            Ds[i] = GetDataAt(xs, ys, sampleIdxOffset, i);
        }
    }
    return Ds;
}