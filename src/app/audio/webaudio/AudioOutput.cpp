#include "AudioOutput.h"

#include <SDL.h>

#include <iostream>

AudioOutput::AudioOutput()
    : m_audioDevice(0), m_sampleRate(48000), m_isPlaying(false), m_time(0) {
    if (SDL_InitSubSystem(SDL_INIT_AUDIO) != 0) {
        std::cerr << "SDL2 Audio init failed: " << SDL_GetError() << std::endl;
    }

    // audioDevice starts uninitialized due to how WebAudio contexts work.
}

AudioOutput::~AudioOutput() {
    SDL_CloseAudioDevice(m_audioDevice);
    SDL_QuitSubSystem(SDL_INIT_AUDIO);
}

void AudioOutput::setBufferCallback(BufferCallback callback) {
    m_bufferCallback = callback;
}

void AudioOutput::startPlaying() {
    if (m_audioDevice == 0) {
        SDL_AudioSpec desiredSpec;
        desiredSpec.freq = 48000;
        desiredSpec.format = AUDIO_F32SYS;
        desiredSpec.channels = 1;
        desiredSpec.samples = 512;
        desiredSpec.callback = &audioCallback;
        desiredSpec.userdata = this;

        m_audioDevice =
            SDL_OpenAudioDevice(nullptr, SDL_FALSE, &desiredSpec, &desiredSpec,
                                SDL_AUDIO_ALLOW_FREQUENCY_CHANGE);
        m_sampleRate = desiredSpec.freq;
    }
    SDL_PauseAudioDevice(m_audioDevice, SDL_FALSE);
    m_isPlaying = true;
}

void AudioOutput::stopPlaying() {
    SDL_PauseAudioDevice(m_audioDevice, SDL_TRUE);
    m_isPlaying = false;
}

bool AudioOutput::isPlaying() const { return m_isPlaying; }

double AudioOutput::sampleRate() const { return m_sampleRate; }

double AudioOutput::time(const int sampleOffset) const {
    return (m_time + sampleOffset) / m_sampleRate;
}

void AudioOutput::audioCallback(void* userdata, Uint8* stream, int lenBytes) {
    auto      self = static_cast<AudioOutput*>(userdata);
    auto      output = reinterpret_cast<float*>(stream);
    const int frameCount = lenBytes / sizeof(float);

    std::vector<double> buffer(frameCount, 0.0f);
    bool                returnedSomething = self->m_bufferCallback(buffer);
    if (returnedSomething) {
        std::copy(buffer.begin(), buffer.end(), output);
    }

    self->m_time += frameCount;
}