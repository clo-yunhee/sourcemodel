#ifndef SOURCEMODEL__AUDIO_BUFFERED_GENERATOR_H
#define SOURCEMODEL__AUDIO_BUFFERED_GENERATOR_H

#include <boost/circular_buffer.hpp>
#include <shared_mutex>
#include <vector>

class AudioTime;

class BufferedGenerator {
   public:
    BufferedGenerator(const AudioTime& time);

    void setBufferLength(int bufferLength);
    void copyBufferTo(std::vector<double>& out);

    void fillBuffer(std::vector<double>& out);

   protected:
    virtual void fillInternalBuffer(std::vector<double>& out) = 0;

    double time(int sampleOffset = 0) const;

   private:
    const AudioTime& m_time;

    std::shared_mutex              m_mutex;
    int                            m_bufferLength;
    boost::circular_buffer<double> m_buffer;
};

#endif  // SOURCEMODEL__AUDIO_BUFFERED_GENERATOR_H