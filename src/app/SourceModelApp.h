#ifndef SOURCEMODEL__SOURCEMODELAPP_H
#define SOURCEMODEL__SOURCEMODELAPP_H

#ifdef USING_RTAUDIO
    #include <RtAudio.h>
#endif

#include "Application.h"
#include "FormantGenerator.h"
#include "GeneratorSpectrum.h"
#include "GlottalFlow.h"
#include "SourceGenerator.h"

#ifdef USING_RTAUDIO
    #include "audio/rtaudio/AudioDevices.h"
    #include "audio/rtaudio/AudioOutput.h"
#endif

#ifdef USING_WEBAUDIO
    #include "audio/webaudio/AudioOutput.h"
#endif

class SourceModelApp : public Application {
   public:
    SourceModelApp(int initialWidth = 800, int initialHeight = 600);
    virtual ~SourceModelApp();

   protected:
    void setupThemeColors(ImGuiStyle& style) override;
    void setupThemeSizes(ImGuiStyle& style) override;

    void renderMenuBar() override;
    void renderMain() override;
    void renderOther() override;

   private:
    void renderSourceParameterControl(GlottalFlowParameter& param, const char* name,
                                      const char* displayName, float itemX);

    void renderFormantParameterControl(int k, float fLabelW, float bLabelW,
                                       float gLabelW);

    bool ScrollableDrag(const char* fieldLabel, float labelW, const char* fieldId,
                        float fieldW, double* value, double min, double max,
                        const char* format, bool autoScale = true);

#ifdef USING_RTAUDIO
    void setAudioOutputDevice(const RtAudio::DeviceInfo& deviceInfo);

    void audioErrorCallback(RtAudioErrorType type, const std::string& errorText);

    RtAudio      m_audio;
    AudioDevices m_audioDevices;
    AudioOutput  m_audioOutput;

    RtAudio::DeviceInfo m_selectedAudioOutputDevice;
#endif

#ifdef USING_WEBAUDIO
    AudioOutput m_audioOutput;
#endif

    // Show message dialog (and optionally add a message)
    void showMessages(const std::string& newLine = "");
    // Render message dialog if needed
    void renderMessages();
    // Mutex because showMessages can be called from a different thread
    std::mutex               m_msgMutex;
    bool                     m_doOpenPopupNextFrame;
    std::vector<std::string> m_messages;

    GlottalFlow       m_glottalFlow;
    SourceGenerator   m_sourceGenerator;
    GeneratorSpectrum m_sourceSpectrum;
    FormantGenerator  m_formantGenerator;
    GeneratorSpectrum m_formantSpectrum;

    double m_pitch;

    bool                m_doBypassFilter;
    std::vector<double> m_intermediateAudioBuffer;

    bool m_doNormalizeFlowPlot;
};

#endif  // SOURCEMODEL__SOURCEMODELAPP_H