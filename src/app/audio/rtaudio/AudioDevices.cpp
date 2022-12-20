#include "AudioDevices.h"

#include <cstring>
#include <iostream>
#include <stdexcept>

AudioDevices::AudioDevices(RtAudio& audio) : m_audio(audio) { refresh(); }

void AudioDevices::refresh() {
    // Copy-on-write for refresh
    const unsigned int               deviceCount = m_audio.getDeviceCount();
    std::vector<unsigned int>        deviceIds = m_audio.getDeviceIds();
    std::vector<RtAudio::DeviceInfo> deviceInfos(deviceCount);

    RtAudio::DeviceInfo defaultInputDevice;
    RtAudio::DeviceInfo defaultOutputDevice;

    for (unsigned int i = 0; i < deviceCount; ++i) {
        deviceInfos[i] = m_audio.getDeviceInfo(deviceIds[i]);

        if (deviceInfos[i].isDefaultInput) {
            defaultInputDevice = deviceInfos[i];
        }
        if (deviceInfos[i].isDefaultOutput) {
            defaultOutputDevice = deviceInfos[i];
        }
    }

    m_deviceIds = deviceIds;
    m_deviceInfos = deviceInfos;
    m_defaultInputDevice = defaultInputDevice;
    m_defaultOutputDevice = defaultOutputDevice;
}

const std::vector<RtAudio::DeviceInfo>& AudioDevices::deviceInfos() const {
    return m_deviceInfos;
}

const RtAudio::DeviceInfo& AudioDevices::device(int index) const {
    return m_deviceInfos[index];
}

const RtAudio::DeviceInfo& AudioDevices::defaultInputDevice() const {
    return m_defaultInputDevice;
}

const RtAudio::DeviceInfo& AudioDevices::defaultOutputDevice() const {
    return m_defaultOutputDevice;
}

const RtAudio::DeviceInfo* AudioDevices::inputDeviceByName(
    const std::string& name) const {
    for (const auto& info : m_deviceInfos) {
        if (info.inputChannels > 0 && info.name == name) {
            return &info;
        }
    }
    return nullptr;
}

const RtAudio::DeviceInfo* AudioDevices::outputDeviceByName(
    const std::string& name) const {
    for (const auto& info : m_deviceInfos) {
        if (info.outputChannels > 0 && info.name == name) {
            return &info;
        }
    }
    return nullptr;
}
