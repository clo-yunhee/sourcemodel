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
        // Relatively high buffer length on web because
        // processing latency with short buffers is really big.
        // Set a high buffer length to avoid constant buffer underruns
        desiredSpec.samples = 2048;
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

Scalar AudioOutput::sampleRate() const { return m_sampleRate; }

Scalar AudioOutput::time(const int sampleOffset) const {
    return (sampleOffset + m_time) / m_sampleRate;
}

uint64_t AudioOutput::timeSamples(const int sampleOffset) const {
    return sampleOffset + m_time;
}

void AudioOutput::audioCallback(void* userdata, Uint8* stream, int lenBytes) {
    auto      self = static_cast<AudioOutput*>(userdata);
    auto      output = reinterpret_cast<float*>(stream);
    const int frameCount = lenBytes / sizeof(float);

    self->m_buffer.resize(frameCount);
    bool returnedSomething = self->m_bufferCallback(self->m_buffer);
    if (returnedSomething) {
        std::copy(self->m_buffer.begin(), self->m_buffer.end(), output);
    }

    self->m_time += frameCount;
}