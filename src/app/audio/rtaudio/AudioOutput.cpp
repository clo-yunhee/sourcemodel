#include "AudioOutput.h"

#include <iostream>

inline void atomic_add(std::atomic<Scalar> &ad, const Scalar plus) {
    Scalar desired, expected = ad.load(std::memory_order_relaxed);
    do {
        desired = expected + plus;
    } while (!ad.compare_exchange_weak(expected, desired));  // seq_cst
}

AudioOutput::AudioOutput(RtAudio &audio) : m_audio(audio), m_timeOffset(0), m_time(0) {
    m_device = audio.getDeviceInfo(audio.getDefaultOutputDevice());
}

AudioOutput::~AudioOutput() {
    if (m_audio.isStreamRunning()) {
        stopPlaying();
    }
    if (m_audio.isStreamOpen()) {
        closeStream();
    }
}

void AudioOutput::setDevice(const RtAudio::DeviceInfo &device) {
    bool restart = m_audio.isStreamRunning();
    if (restart) {
        stopPlaying();
    }
    if (m_audio.isStreamOpen()) {
        closeStream();
    }

    m_device = device;
    std::cout << "AudioOutput: device set: " << device.name << std::endl;

    if (restart) startPlaying();
}

void AudioOutput::setBufferCallback(BufferCallback callback) {
    m_bufferCallback = callback;
}

void AudioOutput::startPlaying() {
    if (!m_audio.isStreamOpen()) {
        openStream();
    }

    m_audio.startStream();

    std::cout << "AudioOutput: playback started" << std::endl;
}

void AudioOutput::stopPlaying() {
    m_audio.stopStream();

    // Set the stop time as the new offset.
    atomic_add(m_timeOffset, m_time / (Scalar)m_audio.getStreamSampleRate());
    // m_timeOffset += m_time / (Scalar)m_audio.getStreamSampleRate();
    m_time = 0;

    std::cout << "AudioOutput: playback stopped" << std::endl;
}

bool AudioOutput::isPlaying() const { return m_audio.isStreamRunning(); }

Scalar AudioOutput::sampleRate() const {
    if (m_audio.isStreamOpen()) {
        return m_audio.getStreamSampleRate();
    } else {
        return m_device.preferredSampleRate;
    }
}

Scalar AudioOutput::time(const int sampleOffset) const {
    return m_timeOffset + (sampleOffset + m_time) / sampleRate();
}

uint64_t AudioOutput::timeSamples(const int sampleOffset) const {
    return m_timeOffset * sampleRate() + sampleOffset + m_time;
}

void AudioOutput::openStream() {
    RtAudio::StreamParameters parameters{};
    parameters.deviceId = m_device.ID;
    parameters.nChannels = m_device.outputChannels;

    RtAudio::StreamOptions options{};
    options.flags = RTAUDIO_NONINTERLEAVED;

    unsigned int bufferFrames{1024};
    m_audio.openStream(&parameters, nullptr, RTAUDIO_FLOAT32,
                       m_device.preferredSampleRate, &bufferFrames, &streamCallback, this,
                       &options);
}

void AudioOutput::closeStream() { m_audio.closeStream(); }

int AudioOutput::streamCallback(void *outputBuffer, void *, unsigned int nBufferFrames,
                                double streamTime, RtAudioStreamStatus status,
                                void *userData) {
    auto self = static_cast<AudioOutput *>(userData);
    auto output = static_cast<float *>(outputBuffer);

    const int channels = self->m_device.outputChannels;

    self->m_buffer.resize(nBufferFrames);
    bool returnedSomething = self->m_bufferCallback(self->m_buffer);
    if (returnedSomething) {
        for (int i = 0; i < nBufferFrames; ++i) {
            self->m_buffer[i] /= channels;
        }
        std::transform(self->m_buffer.begin(), self->m_buffer.end(), &output[0],
                       [](const Scalar x) { return (float)x; });
        for (int c = 1; c < channels; ++c) {
            std::copy(&output[0], &output[nBufferFrames], &output[c * nBufferFrames]);
        }
    }

    self->m_time += nBufferFrames;

    return 0;
}
