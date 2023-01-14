#ifndef SOURCEMODEL__AUDIOTIME_H
#define SOURCEMODEL__AUDIOTIME_H

#include "math/utils.h"

class AudioTime {
   public:
    virtual Scalar   time(int sampleOffset) const = 0;
    virtual uint64_t timeSamples(int sampleOffset) const = 0;
};

#endif  // SOURCEMODEL__AUDIOTIME_H