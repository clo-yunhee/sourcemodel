#ifndef SOURCEMODEL__AUDIOTIME_H
#define SOURCEMODEL__AUDIOTIME_H

class AudioTime {
   public:
    virtual double time(int sampleOffset) const = 0;
};

#endif  // SOURCEMODEL__AUDIOTIME_H