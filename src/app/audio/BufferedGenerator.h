#ifndef SOURCEMODEL__AUDIO_BUFFERED_GENERATOR_H
#define SOURCEMODEL__AUDIO_BUFFERED_GENERATOR_H

#include <boost/circular_buffer.hpp>
#include <mutex>
#include <shared_mutex>
#include <vector>

#include "audio/GainReductionComputer.h"
#include "audio/LookAheadGainReduction.h"
#include "math/utils.h"

class AudioTime;

class BufferedGenerator {
   public:
    BufferedGenerator(const AudioTime& time);

    void     setBufferLength(int bufferLength);
    uint64_t copyBufferTo(std::vector<Scalar>& out);

    bool hasEnoughSamplesSince(uint64_t time, int length);

    void fillBuffer(std::vector<Scalar>& out);

    void   setSampleRate(Scalar fs);
    Scalar sampleRate() const;

    void setNormalized(bool isNorm);
    bool isNormalized() const;

   protected:
    virtual void fillInternalBuffer(std::vector<Scalar>& out) = 0;

    Scalar time(int sampleOffset = 0) const;

    bool   hasSampleRateChanged() const;
    void   ackSampleRateChange();
    Scalar fs() const;

   private:
    void processGainReduction();

    template <typename T>
    using cbso = boost::circular_buffer_space_optimized<T>;

    const AudioTime& m_time;

    std::shared_mutex m_mutex;
    int               m_bufferLength;
    cbso<Scalar>      m_buffer;

    std::vector<Scalar> m_internalBuffer;  // Filled by fillInternalBuffer.

    int          m_delaySamples;  // Compressor delay in samples.
    cbso<Scalar> m_delayBuffer;

    Scalar m_fs;
    bool   m_fsChanged;

    // Anti-aliasing filter.

    // Compressor / limiter stuff.
    bool                   m_isNormalized;
    GainReductionComputer  m_gainReductionComputer;
    LookAheadGainReduction m_lookAheadGainReduction;
    std::vector<float>     m_sidechainSignal;
    std::vector<float>     m_gainReduction;
};

#endif  // SOURCEMODEL__AUDIO_BUFFERED_GENERATOR_H