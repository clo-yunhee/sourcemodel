#ifndef SOURCEMODEL__AUDIO_BUFFERED_GENERATOR_H
#define SOURCEMODEL__AUDIO_BUFFERED_GENERATOR_H

#include <boost/circular_buffer.hpp>
#include <shared_mutex>
#include <vector>

#include "audio/GainReductionComputer.h"
#include "audio/LookAheadGainReduction.h"

class AudioTime;

class BufferedGenerator {
   public:
    BufferedGenerator(const AudioTime& time);

    void setBufferLength(int bufferLength);
    void copyBufferTo(std::vector<double>& out);

    void fillBuffer(std::vector<double>& out);

    void   setSampleRate(double fs);
    double sampleRate() const;

    void setNormalized(bool isNorm);
    bool isNormalized() const;

   protected:
    virtual void fillInternalBuffer(std::vector<double>& out) = 0;

    double time(int sampleOffset = 0) const;

    bool   hasSampleRateChanged() const;
    void   ackSampleRateChange();
    double fs() const;

   private:
    void processGainReduction();

    const AudioTime& m_time;

    std::shared_mutex              m_mutex;
    int                            m_bufferLength;
    boost::circular_buffer<double> m_buffer;

    std::vector<double> m_tempBuffer;
    std::vector<float>  m_sidechainSignal;
    std::vector<float>  m_gainReduction;

    double m_fs;
    bool   m_fsChanged;

    // Anti-aliasing filter.

    // Compressor / limiter stuff.
    bool                   m_isNormalized;
    GainReductionComputer  m_gainReductionComputer;
    LookAheadGainReduction m_lookAheadGainReduction;
};

#endif  // SOURCEMODEL__AUDIO_BUFFERED_GENERATOR_H