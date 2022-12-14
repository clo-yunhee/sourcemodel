#include "SourceModelApp.h"

#include <implot.h>
#include <implot_internal.h>

#include <algorithm>
#include <cmath>
#include <iostream>

#include "math/LTTB.h"
#include "math/utils.h"

SourceModelApp::SourceModelApp(const int initialWidth, const int initialHeight)
    : Application("SourceModel", initialWidth, initialHeight),
#ifdef USING_RTAUDIO
      m_audioDevices(m_audio),
      m_audioOutput(m_audio),
      m_selectedAudioOutputDevice(m_audioDevices.defaultOutputDevice()),
#endif
      m_sourceGenerator(m_audioOutput, m_glottalFlow),
      m_sourceSpectrum(&m_sourceGenerator),
      m_formantGenerator(m_audioOutput, m_intermediateAudioBuffer),
      m_formantSpectrum(&m_formantGenerator),
      m_showAdvancedSourceParams(false),
      m_doBypassFilter(false),
      m_doNormalizeFlowPlot(true) {
    ImPlot::CreateContext();

    m_audioOutput.setBufferCallback([this](std::vector<double>& out) {
        m_intermediateAudioBuffer.resize(out.size());
        m_sourceGenerator.fillBuffer(m_intermediateAudioBuffer);
        if (m_doBypassFilter) {
            std::copy(m_intermediateAudioBuffer.begin(), m_intermediateAudioBuffer.end(),
                      out.begin());
            m_formantGenerator.fillBuffer(m_intermediateAudioBuffer);
            // Compensate gain (the source is usually *perceived* louder so lower it)
            for (auto& x : out) x *= 0.25;
        } else {
            m_formantGenerator.fillBuffer(out);
        }
        return true;
    });

    m_glottalFlow.setSampleCount(1024);
    m_glottalFlow.setModelType(GlottalFlowModel_LF);  // Default model to LF.

    m_sourceSpectrum.setSmoothing(0.2);
    m_sourceSpectrum.setTransformSize(6144);

    m_formantSpectrum.setSmoothing(0.2);
    m_formantSpectrum.setTransformSize(6144);

    m_formantGenerator.spectrum().setSize(4096);

    m_sourceGenerator.setNormalized(true);
    m_formantGenerator.setNormalized(false);

#ifdef USING_RTAUDIO
    setAudioOutputDevice(m_audioDevices.defaultOutputDevice());
#endif

    m_glottalFlow.parameters().Oq.valueChanged.connect(
        &SourceGenerator::handleParamChanged, &m_sourceGenerator);
    m_glottalFlow.parameters().am.valueChanged.connect(
        &SourceGenerator::handleParamChanged, &m_sourceGenerator);
    m_glottalFlow.parameters().Qa.valueChanged.connect(
        &SourceGenerator::handleParamChanged, &m_sourceGenerator);
    m_glottalFlow.parameters().usingRdChanged.connect(
        &SourceGenerator::handleUsingRdChanged, &m_sourceGenerator);
    m_glottalFlow.parameters().Rd.valueChanged.connect(
        &SourceGenerator::handleParamChanged, &m_sourceGenerator);

    m_glottalFlow.modelTypeChanged.connect(&SourceGenerator::handleModelChanged,
                                           &m_sourceGenerator);
}

SourceModelApp::~SourceModelApp() { ImPlot::DestroyContext(); }

void SourceModelApp::setupThemeColors(ImGuiStyle& style) {
    if (isDarkTheme()) {
        ImGui::StyleColorsDark(&style);
        style.Colors[ImGuiCol_Text] = ImVec4(1.00f, 1.00f, 1.00f, 0.98f);
        style.Colors[ImGuiCol_TextDisabled] = ImVec4(0.70f, 0.70f, 0.70f, 0.98f);
        style.Colors[ImGuiCol_TextSelectedBg] = ImVec4(1.00f, 1.00f, 1.00f, 0.98f);
        ImPlot::StyleColorsDark();
    } else {
        ImGui::StyleColorsLight(&style);
        style.Colors[ImGuiCol_Text] = ImVec4(0.00f, 0.00f, 0.00f, 0.98f);
        style.Colors[ImGuiCol_TextDisabled] = ImVec4(0.30f, 0.30f, 0.30f, 0.98f);
        style.Colors[ImGuiCol_TextSelectedBg] = ImVec4(0.00f, 0.00f, 0.00f, 0.98f);
        ImPlot::StyleColorsLight();
    }
}

void SourceModelApp::setupThemeSizes(ImGuiStyle& style) {
    style.ItemSpacing = ImVec2(12, 6);
    style.WindowRounding = 3.0f;
}

void SourceModelApp::renderMenuBar() {
    if (!m_audioOutput.isPlaying()) {
        if (ImGui::MenuItem("\uf04b")) {
            m_audioOutput.startPlaying();
        }
    } else {
        if (ImGui::MenuItem("\uf04c")) {
            m_audioOutput.stopPlaying();
        }
    }

    ImGui::Separator();

    // No point in showing audio settings on Web
    if (ImGui::BeginMenu("Audio settings")) {
#ifdef USING_RTAUDIO
        if (ImGui::BeginMenu("Playback device")) {
            m_audioDevices.refresh();

            for (const auto& deviceInfo : m_audioDevices.deviceInfos()) {
                // If it's an output device
                if (deviceInfo.outputChannels > 0) {
                    bool isSelected = (m_selectedAudioOutputDevice.ID == deviceInfo.ID);
                    if (ImGui::MenuItem(deviceInfo.name.c_str(), nullptr, isSelected)) {
                        if (!isSelected) {
                            setAudioOutputDevice(deviceInfo);
                        }
                    }
                }
            }
            ImGui::EndMenu();
        }
#endif

        char line[64];
        snprintf(line, 64, "Sample rate: %d Hz", (int)m_audioOutput.sampleRate());
        ImGui::MenuItem(line, nullptr, false, false);
        ImGui::EndMenu();
    }

    ImGui::Separator();
}

void SourceModelApp::renderMain() {
    ImGuiIO&    io = ImGui::GetIO();
    ImGuiStyle& style = ImGui::GetStyle();

    if (m_glottalFlow.isDirty()) {
        m_glottalFlow.updateSamples();
    }

    m_sourceSpectrum.update();
    m_formantSpectrum.update();
    m_formantGenerator.updateSpectrumIfNeeded();

    const float childLWidth = std::max(ImGui::GetContentRegionAvail().x / 3, 25 * em());
    ImGui::BeginChild("ChildL", ImVec2(childLWidth, -1));

    ImGui::BeginGroupPanel("Glottal flow model", ImVec2(-1, 0));

    const float itemX =
        ImGui::CalcTextSize("Oq \u2208 [0.000, 0.000]").x + style.ItemSpacing.x;

    ImGui::NewLine();
    ImGui::SameLine(itemX);
    ImGui::AlignTextToFramePadding();
    ImGui::TextUnformatted("E\u2080 = 1 ; T\u2080 = 1");
    ImGui::Spacing();

    if (ImGui::Checkbox("Show advanced params", &m_showAdvancedSourceParams)) {
        m_glottalFlow.parameters().setUsingRd(!m_showAdvancedSourceParams);
    }

    if (m_showAdvancedSourceParams) {
        ImGui::AlignTextToFramePadding();
        ImGui::TextUnformatted("Model type");
        ImGui::SameLine(itemX);
        int type = (int)m_glottalFlow.modelType();
        ImGui::SetNextItemWidth(15 * em());
        if (ImGui::Combo("##gfm_type", &type, GlottalFlowModel_NAMES)) {
            m_glottalFlow.setModelType((GlottalFlowModelType)type);
        }

        renderSourceParameterControl(m_glottalFlow.parameters().Oq, "Oq", "Oq", itemX);
        renderSourceParameterControl(m_glottalFlow.parameters().am, "am", "\u03B1\u2098",
                                     itemX);
        renderSourceParameterControl(m_glottalFlow.parameters().Qa, "Qa", "Q\u2090",
                                     itemX);
    } else {
        ImGui::AlignTextToFramePadding();
        ImGui::NewLine();
        ImGui::SameLine(itemX);
        ImGui::TextUnformatted("LF model with Rd waveshape parameter.");

        double Rd = m_glottalFlow.parameters().Rd.value();

        // By 0.01 if < 0.1 else 0.1
        const int precision = Rd <= 0.1 ? -3 : -2;

        if (ScrollableDrag("Rd \u2208 [0.01, 6]", itemX, "##param_Rd", 15 * em(), &Rd,
                           0.01, 6.0, "%g", false, precision)) {
            m_glottalFlow.parameters().Rd.setValue(Rd);
        }

        ImGui::TextWrapped(
            "Low, mid or high values of Rd correspond to tense/pressed, modal/normal or "
            "relaxed/breathy voice qualities respectively.");
    }

    ImGui::EndGroupPanel();  // Glottal source modelling

    ImGui::BeginGroupPanel("Glottal flow waveform", ImVec2(-1, 0));

    const float gfmPlotHeight = ImGui::GetContentRegionAvail().y - 2 * em();

    if (ImPlot::BeginPlot("##gfm_timeplot", ImVec2(-1, gfmPlotHeight),
                          ImPlotFlags_NoFrame | ImPlotFlags_NoMenus)) {
        ImPlot::SetupAxis(ImAxis_X1, "Time", ImPlotAxisFlags_NoMenus);
        ImPlot::SetupAxisLimitsConstraints(ImAxis_X1, 0, 1);
        ImPlot::SetupAxisLimits(ImAxis_X1, 0, 1, ImPlotCond_Once);

        ImPlot::SetupAxis(ImAxis_Y1, "", ImPlotAxisFlags_NoMenus);
        ImPlot::SetupAxisLimitsConstraints(ImAxis_Y1, -1.02, 1.02);
        ImPlot::SetupAxisLimits(ImAxis_Y1, -1.02, 1.02, ImPlotCond_Once);

        ImPlot::SetupLegend(ImPlotLocation_NorthEast, ImPlotLegendFlags_Outside);

        const double x1 = ImPlot::GetPlotLimits().X.Min;
        const double x2 = ImPlot::GetPlotLimits().X.Max;

        const double xPerPixel = (x2 - x1) / ImPlot::GetPlotSize().x;

        const int N = m_glottalFlow.sampleCount();
        const int i1 = std::clamp((int)std::floor(x1 * (N - 1)), 0, N - 1);
        const int i2 = std::clamp((int)std::floor(x2 * (N - 1)) + 1, 0, N - 1);

        const double nPerPixel = N * xPerPixel;

        const int dsN = (int)std::round((i2 - i1 + 1) / nPerPixel);

        auto g = downsampleLTTB(m_glottalFlow.t(), m_glottalFlow.g(),
                                m_glottalFlow.sampleCount(), dsN, i1, i2);
        auto dg = downsampleLTTB(m_glottalFlow.t(), m_glottalFlow.dg(),
                                 m_glottalFlow.sampleCount(), dsN, i1, i2);

        const double gScale =
            (m_doNormalizeFlowPlot ? 1.0 / m_glottalFlow.gAmplitude() : 1);
        const double dgScale =
            (m_doNormalizeFlowPlot ? 1.0 / m_glottalFlow.dgAmplitude() : 1);

        // Scale both dg and g by gAmplitude.
        for (auto& p : g) p.y *= gScale;
        for (auto& p : dg) p.y *= dgScale;

        // Always include the actual min and max values. (not the downsampled ones)
        bool dgMinIns = false;
        bool dgMaxIns = false;
        for (auto it = dg.begin(); it != dg.end() && !dgMinIns && !dgMaxIns; ++it) {
            if (!dgMinIns && it->x > m_glottalFlow.dgMin().first) {
                it = dg.insert(it, {m_glottalFlow.dgMin().first,
                                    m_glottalFlow.dgMin().second * dgScale});
                dgMinIns = true;
            }
            if (!dgMaxIns && it->x > m_glottalFlow.dgMax().first) {
                it = dg.insert(it, {m_glottalFlow.dgMax().first,
                                    m_glottalFlow.dgMax().second * dgScale});
                dgMaxIns = true;
            }
        }

        bool gMinIns = false;
        bool gMaxIns = false;
        for (auto it = g.begin(); it != g.end() && !gMinIns && !gMaxIns; ++it) {
            if (!gMinIns && it->x > m_glottalFlow.gMin().first) {
                it = g.insert(it, {m_glottalFlow.gMin().first,
                                   m_glottalFlow.gMin().second * gScale});
                gMinIns = true;
            }
            if (!gMaxIns && it->x > m_glottalFlow.gMax().first) {
                it = g.insert(it, {m_glottalFlow.gMax().first,
                                   m_glottalFlow.gMax().second * gScale});
                gMaxIns = true;
            }
        }

        ImPlot::PushStyleVar(ImPlotStyleVar_LineWeight, 1.75f * contentScale());

        // ImPlot::PlotLineG("g(t)", plotGetterLTTB, g.Data, g.size());
        // ImPlot::PlotLineG("dg(t)", plotGetterLTTB, dg.Data, dg.size());

        ImPlot::PlotLine("g(t)", m_glottalFlow.t(), m_glottalFlow.g(),
                         m_glottalFlow.sampleCount());
        ImPlot::PlotLine("dg(t)", m_glottalFlow.t(), m_glottalFlow.dg(),
                         m_glottalFlow.sampleCount());

        ImPlot::PopStyleVar(ImPlotStyleVar_LineWeight);

        ImPlot::EndPlot();  // ##gfm_timeplot
    }

    ImGui::Checkbox("Normalize?", &m_doNormalizeFlowPlot);

    ImGui::EndGroupPanel();  // Glottal flow waveform

    ImGui::EndChild();  // ChildL

    ImGui::SameLine();

    ImGui::BeginChild("ChildR", ImVec2(-1, -1));

    ImGui::BeginGroupPanel("Synthesizer", ImVec2(-1, 0));

    constexpr const char* f0Label = "f\u2080";
    const float           f0LabelW = ImGui::CalcTextSize(f0Label).x + style.ItemSpacing.x;
    double                pitch = m_sourceGenerator.pitch();
    if (ScrollableDrag(f0Label, f0LabelW, "##param_f0", 15 * em(), &pitch, 16.0, 1000.0,
                       "%g Hz")) {
        m_sourceGenerator.setPitch(pitch);
    }

    const float fLabelW = ImGui::CalcTextSize("f\u2081").x + style.ItemSpacing.x;
    const float bLabelW = ImGui::CalcTextSize("b\u2081").x + style.ItemSpacing.x;
    const float gLabelW = ImGui::CalcTextSize("G\u2081").x + style.ItemSpacing.x;

    ImGui::SameLine(f0LabelW + 15 * em() + bLabelW + style.ItemSpacing.x);

    ImGui::Checkbox("Bypass filter?", &m_doBypassFilter);

    for (int k = 0; k < FormantGenerator::kNumFormants; ++k) {
        renderFormantParameterControl(k, fLabelW, bLabelW, gLabelW);
    }

    ImGui::EndGroupPanel();  // Synthesizer

    ImGui::BeginGroupPanel("Spectrum", ImVec2(-1, 0));

    const float spectrumPlotHeight = ImGui::GetContentRegionAvail().y - 0.5f * em();

    if (ImPlot::BeginPlot("##specplot", ImVec2(-1, spectrumPlotHeight),
                          ImPlotFlags_NoFrame)) {
        ImPlot::SetupAxis(ImAxis_X1, "Frequency [Hz]", ImPlotAxisFlags_None);
        ImPlot::SetupAxisScale(ImAxis_X1, ImPlotScale_Log10);
        ImPlot::SetupAxisLimitsConstraints(ImAxis_X1, 1, m_audioOutput.sampleRate() / 2);
        ImPlot::SetupAxisLimits(ImAxis_X1, 50, 12000);

        ImPlot::SetupAxis(ImAxis_Y1, "Intensity [dB]", ImPlotAxisFlags_None);
        ImPlot::SetupAxisLimits(ImAxis_Y1, -40, 30);

        ImPlot::SetupLegend(ImPlotLocation_NorthEast);

        ImPlot::PushStyleVar(ImPlotStyleVar_LineWeight, 2.0f * contentScale());

        ImPlot::PlotLine("Glottal source", m_sourceSpectrum.frequencies(),
                         m_sourceSpectrum.magnitudesDb(), m_sourceSpectrum.binCount());

        ImPlot::PopStyleVar(ImPlotStyleVar_LineWeight);

        ImPlot::PushStyleVar(ImPlotStyleVar_LineWeight, 2.0f * contentScale());

        ImPlot::PlotLine("Filtered source", m_formantSpectrum.frequencies(),
                         m_formantSpectrum.magnitudesDb(), m_formantSpectrum.binCount());

        ImPlot::PopStyleVar(ImPlotStyleVar_LineWeight);

        ImPlot::PushStyleVar(ImPlotStyleVar_LineWeight, 2.0f * contentScale());

        ImPlot::PlotLine("Filter response", m_formantGenerator.spectrum().frequencies(),
                         m_formantGenerator.spectrum().magnitudesDb(),
                         m_formantGenerator.spectrum().binCount());

        ImPlot::PopStyleVar(ImPlotStyleVar_LineWeight);

        ImPlot::EndPlot();  // ##specplot
    }

    ImGui::EndGroupPanel();  // Spectrum

    ImGui::EndChild();  // ChildR
}

void SourceModelApp::renderOther() {}

void SourceModelApp::renderSourceParameterControl(GlottalFlowParameter& param,
                                                  const char*           name,
                                                  const char*           displayName,
                                                  const float           itemX) {
    if (param.isFixed()) {
        ImGui::AlignTextToFramePadding();
        ImGui::Text("%s = %g", displayName, param.value());
    } else {
        static constexpr int bufferLength = 32;
        static char          fieldLabel[bufferLength];
        static char          fieldId[bufferLength];

        snprintf(fieldLabel, bufferLength, "%s \u2208 [%g, %g]", displayName, param.min(),
                 param.max());
        snprintf(fieldId, bufferLength, "##param_%s", name);

        double value = param.value();
        if (ScrollableDrag(fieldLabel, itemX, fieldId, 15 * em(), &value, param.min(),
                           param.max(), "%g")) {
            if (!fuzzyEquals(param.value(), value)) {
                param.setValue(value);
            }
        }
    }
}

void SourceModelApp::renderFormantParameterControl(const int k, const float fLabelW,
                                                   const float bLabelW,
                                                   const float gLabelW) {
    constexpr const char* subscriptK[5] = {"\u2081", "\u2082", "\u2083", "\u2084",
                                           "\u2085"};

    static constexpr int bufferLength = 32;
    static char          fieldLabel[bufferLength];
    static char          fieldId[bufferLength];

    snprintf(fieldLabel, bufferLength, "f%s", subscriptK[k]);
    snprintf(fieldId, bufferLength, "##param_F%d", (k + 1));

    double freq = m_formantGenerator.frequency(k);
    if (ScrollableDrag(fieldLabel, fLabelW, fieldId, 15 * em(), &freq, 200.0, 6000.0,
                       "%g Hz")) {
        m_formantGenerator.setFrequency(k, freq);
    }

    ImGui::SameLine();

    snprintf(fieldLabel, bufferLength, "b%s", subscriptK[k]);
    snprintf(fieldId, bufferLength, "##param_B%d", (k + 1));

    double band = m_formantGenerator.bandwidth(k);
    if (ScrollableDrag(fieldLabel, bLabelW, fieldId, 7.5 * em(), &band, 10.0, 600.0,
                       "%g Hz")) {
        m_formantGenerator.setBandwidth(k, band);
    }
}

bool SourceModelApp::ScrollableDrag(const char* fieldLabel, const float labelW,
                                    const char* fieldId, const float fieldW,
                                    double* value, double min, double max,
                                    const char* format, const bool autoScale,
                                    const int manualPrecision) {
    const int precision = autoScale
                            ? std::clamp((int)std::floor(log10(fabs(*value))), -3, 5)
                            : manualPrecision;

    ImGui::BeginGroup();
    ImGui::AlignTextToFramePadding();
    ImGui::TextUnformatted(fieldLabel);
    ImGui::SameLine(labelW);
    ImGui::SetNextItemWidth(fieldW);
    if (ImGui::DragDouble(fieldId, value, pow(10, precision - 1), min, max, format)) {
        ImGui::EndGroup();
        return true;
    }
    ImGui::EndGroup();

    ImGuiIO& io = ImGui::GetIO();
    if (ImGui::IsItemHovered() && io.MouseWheel != 0) {
        *value =
            std::clamp(*value + 2 * pow(10, precision - 1) * io.MouseWheel, min, max);
        return true;
    }
    return false;
}

void SourceModelApp::updateDownscaledPlot(const int count, const int start,
                                          const int end) {
    const int     N = m_glottalFlow.sampleCount();
    const double* t = m_glottalFlow.t();

    const double* g = m_glottalFlow.g();
    const auto&   gMin = m_glottalFlow.gMin();
    const auto&   gMax = m_glottalFlow.gMax();
    const double  gAmp = m_glottalFlow.gAmplitude();

    const double* dg = m_glottalFlow.dg();
    const auto&   dgMin = m_glottalFlow.dgMin();
    const auto&   dgMax = m_glottalFlow.dgMax();
    const double  dgAmp = m_glottalFlow.dgAmplitude();

    m_downsampledCount = count;
    m_downsampledStart = start;
    m_downsampledEnd = end;

    m_gDownsampled = downsampleLTTB(t, g, N, count, start, end);
    m_dgDownsampled = downsampleLTTB(t, dg, N, count, start, end);

    const double gScale = (m_doNormalizeFlowPlot ? 1.0 / gAmp : 1);
    const double dgScale = (m_doNormalizeFlowPlot ? 1.0 / dgAmp : 1);

    bool gMinInserted = false;
    bool gMaxInserted = false;
    for (auto it = m_gDownsampled.begin();
         it != m_gDownsampled.end() && !gMinInserted && !gMaxInserted; ++it) {
        if (!gMinInserted && it->x > gMin.first) {
            it = m_gDownsampled.insert(it, {gMin.first, gMin.second});
            gMinInserted = true;
        }
        if (!gMaxInserted && it->x > gMax.first) {
            it = m_gDownsampled.insert(it, {gMax.first, gMax.second});
            gMaxInserted = true;
        }
    }

    bool dgMinInserted = false;
    bool dgMaxInserted = false;
    for (auto it = m_dgDownsampled.begin();
         it != m_dgDownsampled.end() && !dgMinInserted && !dgMaxInserted; ++it) {
        if (!dgMinInserted && it->x > dgMin.first) {
            it = m_dgDownsampled.insert(it, {dgMin.first, dgMin.second});
            dgMinInserted = true;
        }
        if (!dgMaxInserted && it->x > dgMax.first) {
            it = m_dgDownsampled.insert(it, {dgMax.first, dgMax.second});
            dgMaxInserted = true;
        }
    }
}

void SourceModelApp::showMessages(const std::string& newLine) {
    std::unique_lock lock(m_msgMutex);

    if (!newLine.empty()) {
        m_messages.push_back(newLine);
    }
    m_doOpenPopupNextFrame = true;
}

void SourceModelApp::renderMessages() {
    std::unique_lock lock(m_msgMutex);

    if (m_doOpenPopupNextFrame) {
        m_doOpenPopupNextFrame = false;
        ImGui::OpenPopup("Messages");
    }

    // Pass a bool* to BeginPopupModal to show a close button.
    // It isn't used anywhere else.
    bool pOpenUnused;
    if (ImGui::BeginPopupModal("Messages", &pOpenUnused,
                               ImGuiWindowFlags_AlwaysAutoResize)) {
        for (auto it = m_messages.begin(); it != m_messages.end(); ++it) {
            ImGui::TextWrapped("%s", it->c_str());
        }

        if (ImGui::Button("Close")) {
            ImGui::CloseCurrentPopup();
        }
        if (ImGui::Button("Clear and close")) {
            m_messages.clear();
            ImGui::CloseCurrentPopup();
        }
        ImGui::EndPopup();
    }
}

#ifdef USING_RTAUDIO
void SourceModelApp::setAudioOutputDevice(const RtAudio::DeviceInfo& deviceInfo) {
    m_selectedAudioOutputDevice = deviceInfo;
    m_sourceGenerator.setSampleRate(m_audioOutput.sampleRate());
    m_sourceSpectrum.setSampleRate(m_audioOutput.sampleRate());
    m_formantGenerator.setSampleRate(m_audioOutput.sampleRate());
    m_formantSpectrum.setSampleRate(m_audioOutput.sampleRate());
    m_audioOutput.setDevice(deviceInfo);
}

void SourceModelApp::audioErrorCallback(RtAudioErrorType   type,
                                        const std::string& errorText) {
    showMessages("RtAudio: " + errorText);
}
#endif