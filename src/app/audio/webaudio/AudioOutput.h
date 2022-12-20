#ifndef SOURCEMODEL__WEBAUDIO_AUDIOOUTPUT_H
#define SOURCEMODEL__WEBAUDIO_AUDIOOUTPUT_H

#include <SDL_audio.h>

#include <atomic>
#include <cstdint>
#include <functional>
#include <vector>

#include "../AudioTime.h"

class AudioOutput : public AudioTime {
   public:
    using BufferCallback = std::function<bool(std::vector<double> &)>;

    AudioOutput();
    ~AudioOutput();

    void setBufferCallback(BufferCallback callback);

    void startPlaying();
    void stopPlaying();

    bool isPlaying() const;

    double sampleRate() const;
    double time(int sampleOffset) const override;

   private:
    static void audioCallback(void *userdata, Uint8 *stream, int len);

    SDL_AudioDeviceID m_audioDevice;
    double            m_sampleRate;

    bool m_isPlaying;

    BufferCallback m_bufferCallback;

    std::atomic_uint64_t m_time;
};

#endif  // SOURCEMODEL__WEBAUDIO_AUDIOOUTPUT_H