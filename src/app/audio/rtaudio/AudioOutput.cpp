#include "AudioOutput.h"

#include <iostream>

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
    m_timeOffset += m_time / (double)m_audio.getStreamSampleRate();
    m_time = 0;

    std::cout << "AudioOutput: playback stopped" << std::endl;
}

bool AudioOutput::isPlaying() const { return m_audio.isStreamRunning(); }

double AudioOutput::sampleRate() const {
    if (m_audio.isStreamOpen()) {
        return m_audio.getStreamSampleRate();
    } else {
        return m_device.preferredSampleRate;
    }
}

double AudioOutput::time(const int sampleOffset) const {
    return m_timeOffset + (m_time + sampleOffset) / sampleRate();
}

void AudioOutput::openStream() {
    RtAudio::StreamParameters parameters{};
    parameters.deviceId = m_device.ID;
    parameters.nChannels = m_device.outputChannels;

    RtAudio::StreamOptions options{};
    options.flags = RTAUDIO_NONINTERLEAVED;

    unsigned int bufferFrames{256};
    m_audio.openStream(&parameters, nullptr, RTAUDIO_FLOAT64,
                       m_device.preferredSampleRate, &bufferFrames, &streamCallback, this,
                       &options);
    m_buffer.resize(bufferFrames);
}

void AudioOutput::closeStream() { m_audio.closeStream(); }

int AudioOutput::streamCallback(void *outputBuffer, void *, unsigned int nBufferFrames,
                                double streamTime, RtAudioStreamStatus status,
                                void *userData) {
    auto self = static_cast<AudioOutput *>(userData);
    auto output = static_cast<double *>(outputBuffer);

    const int channels = self->m_device.outputChannels;

    bool returnedSomething = self->m_bufferCallback(self->m_buffer);
    if (returnedSomething) {
        for (int i = 0; i < nBufferFrames; ++i) {
            self->m_buffer[i] /= channels;
        }
        for (int c = 0; c < channels; ++c) {
            std::copy(self->m_buffer.begin(), self->m_buffer.end(),
                      &output[c * nBufferFrames]);
        }
    }

    self->m_time += nBufferFrames;

    return 0;
}
