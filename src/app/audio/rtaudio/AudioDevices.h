#ifndef SOURCEMODEL__RTAUDIO_AUDIODEVICES_H
#define SOURCEMODEL__RTAUDIO_AUDIODEVICES_H

#include <RtAudio.h>

#include <string>
#include <vector>

class AudioDevices {
   public:
    AudioDevices(RtAudio &audio);

    void refresh();

    const std::vector<RtAudio::DeviceInfo> &deviceInfos() const;

    const RtAudio::DeviceInfo &device(int index) const;

    const RtAudio::DeviceInfo &defaultInputDevice() const;
    const RtAudio::DeviceInfo &defaultOutputDevice() const;

    const RtAudio::DeviceInfo *inputDeviceByName(const std::string &name) const;
    const RtAudio::DeviceInfo *outputDeviceByName(const std::string &name) const;

   private:
    RtAudio &m_audio;

    std::vector<unsigned int>        m_deviceIds;
    std::vector<RtAudio::DeviceInfo> m_deviceInfos;

    RtAudio::DeviceInfo m_defaultInputDevice;
    RtAudio::DeviceInfo m_defaultOutputDevice;
};

#endif  // SOURCEMODEL__RTAUDIO_AUDIODEVICES_H