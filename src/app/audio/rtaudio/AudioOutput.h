#ifndef SOURCEMODEL__RTAUDIO_AUDIOOUTPUT_H
#define SOURCEMODEL__RTAUDIO_AUDIOOUTPUT_H

#include <RtAudio.h>

#include <atomic>
#include <cstdint>
#include <functional>
#include <mutex>
#include <queue>
#include <vector>

#include "../AudioTime.h"
#include "AudioDevices.h"

class AudioOutput : public AudioTime {
   public:
    using BufferCallback = std::function<bool(std::vector<double> &)>;

    AudioOutput(RtAudio &audio);
    ~AudioOutput();

    void setDevice(const RtAudio::DeviceInfo &device);
    void setBufferCallback(BufferCallback callback);

    void startPlaying();
    void stopPlaying();

    bool isPlaying() const;

    double sampleRate() const;
    double time(int sampleOffset) const override;

   private:
    void openStream();
    void closeStream();

    static int streamCallback(void *outputBuffer, void *, unsigned int nBufferFrames,
                              double streamTime, RtAudioStreamStatus status,
                              void *userData);

    RtAudio            &m_audio;
    RtAudio::DeviceInfo m_device;

    BufferCallback      m_bufferCallback;
    std::vector<double> m_buffer;

    std::atomic<double>  m_timeOffset;
    std::atomic_uint64_t m_time;
};

#endif  // SOURCEMODEL__PORTAUDIO_AUDIOOUTPUT_H