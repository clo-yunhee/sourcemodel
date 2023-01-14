#include "SourceModelApp.h"

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
      m_downsampledCount(0),
      m_downsampledStart(-1),
      m_downsampledEnd(-1),
      m_showAdvancedSourceParams(false),
      m_doBypassFilter(false),
      m_doNormalizeFlowPlot(true),
      m_spectrumFrequencyScale(FrequencyScale_Mel) {
    ImPlot::CreateContext();

    m_audioOutput.setBufferCallback([this](std::vector<Scalar>& out) {
        m_intermediateAudioBuffer.resize(out.size());
        m_sourceGenerator.fillBuffer(m_intermediateAudioBuffer);
        if (m_doBypassFilter) {
            std::copy(m_intermediateAudioBuffer.begin(), m_intermediateAudioBuffer.end(),
                      out.begin());
            m_formantGenerator.fillBuffer(m_intermediateAudioBuffer);
            // Compensate gain (the source is usually *perceived* louder so lower it)
            for (auto& x : out) x *= 0.25_f;
        } else {
            m_formantGenerator.fillBuffer(out);
        }
        return true;
    });

    m_glottalFlow.setSampleCount(1024);
    m_glottalFlow.setModelType(GlottalFlowModel_LF);  // Default model to LF.

    m_sourceSpectrum.setResponseTime(0.025);
    m_formantSpectrum.setResponseTime(0.025);

    m_sourceSpectrum.setTransformSize(4096);
    m_formantSpectrum.setTransformSize(4096);
    m_formantGenerator.spectrum().setSize(4096);

    m_sourceGenerator.setNormalized(true);
    m_formantGenerator.setNormalized(false);

#ifdef USING_RTAUDIO
    setAudioOutputDevice(m_audioDevices.defaultOutputDevice());
#else
    m_sourceGenerator.setSampleRate(m_audioOutput.sampleRate());
    m_sourceSpectrum.setSampleRate(m_audioOutput.sampleRate());
    m_formantGenerator.setSampleRate(m_audioOutput.sampleRate());
    m_formantSpectrum.setSampleRate(m_audioOutput.sampleRate());
    m_formantGenerator.spectrum().setSampleRate(m_audioOutput.sampleRate());
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
    if (m_glottalFlow.isDirty()) {
        m_glottalFlow.updateSamples();
        // Also update downsampled plot (only if it hasn't been initialized before).
        if (m_downsampledCount != 0) {
            updateDownscaledPlot(m_downsampledCount, m_downsampledStart,
                                 m_downsampledEnd);
        }
    }

    ImGuiIO&    io = ImGui::GetIO();
    ImGuiStyle& style = ImGui::GetStyle();

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

        Scalar Rd = m_glottalFlow.parameters().Rd.value();

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

        if (m_downsampledCount != N || m_downsampledStart != i1 ||
            m_downsampledEnd != i2) {
            updateDownscaledPlot(N, i1, i2);
        }

        ImPlot::PushStyleVar(ImPlotStyleVar_LineWeight, 1.75f * contentScale());

        ImPlot::PlotLineG("g(t)", plotGetterLTTB, m_gDownsampled.Data,
                          m_gDownsampled.size());
        ImPlot::PlotLineG("dg(t)", plotGetterLTTB, m_dgDownsampled.Data,
                          m_dgDownsampled.size());

        ImPlot::PopStyleVar(ImPlotStyleVar_LineWeight);

        ImPlot::EndPlot();  // ##gfm_timeplot
    }

    if (ImGui::Checkbox("Normalize?", &m_doNormalizeFlowPlot)) {
        updateDownscaledPlot(m_downsampledCount, m_downsampledStart, m_downsampledEnd);
    }

    ImGui::EndGroupPanel();  // Glottal flow waveform

    ImGui::EndChild();  // ChildL

    ImGui::SameLine();

    ImGui::BeginChild("ChildR", ImVec2(-1, -1));

    ImGui::BeginGroupPanel("Synthesizer", ImVec2(childLWidth, 0));

    constexpr const char* f0Label = "f\u2080";
    const float           f0LabelW = ImGui::CalcTextSize(f0Label).x + style.ItemSpacing.x;
    Scalar                pitch = m_sourceGenerator.pitch();
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

    ImGui::SameLine();

    ImGui::BeginGroupPanel("Spectrum settings", ImVec2(childLWidth, 0));

    ImGui::SetNextItemWidth(10 * em());
    if (ImGui::BeginCombo("Frequency scale",
                          FrequencyScale_Name(m_spectrumFrequencyScale))) {
        for (int i = 0; i < FrequencyScale_COUNT; ++i) {
            const auto scale = static_cast<FrequencyScale>(i);
            if (ImGui::Selectable(FrequencyScale_Name(scale),
                                  scale == m_spectrumFrequencyScale)) {
                m_spectrumFrequencyScale = scale;
            }
        }
        ImGui::EndCombo();
    }

    ImGui::SetNextItemWidth(10 * em());
    const int nfft = m_sourceSpectrum.transformSize();
    if (ImGui::BeginCombo("Transform size", std::to_string(nfft).c_str())) {
        for (int N = 512; N <= 32768; N *= 2) {
            if (ImGui::Selectable(std::to_string(N).c_str(), N == nfft)) {
                m_sourceSpectrum.setTransformSize(N);
                m_formantSpectrum.setTransformSize(N);
            }
        }
        ImGui::EndCombo();
    }

    ImGui::EndGroupPanel();  // Spectrum settings

    ImGui::BeginGroupPanel("Spectrum", ImVec2(-1, 0));

    const float spectrumPlotHeight = ImGui::GetContentRegionAvail().y - 0.5f * em();

    if (ImPlot::BeginPlot("##specplot", ImVec2(-1, spectrumPlotHeight),
                          ImPlotFlags_NoFrame)) {
        ImPlot::SetupAxis(ImAxis_X1, "Frequency [Hz]", ImPlotAxisFlags_None);
        ImPlot::SetupAxisLimitsConstraints(ImAxis_X1, 8, m_audioOutput.sampleRate() / 2);
        ImPlot::SetupAxisLimits(ImAxis_X1, 60, 8000);

        ImPlot::SetupAxisScale(ImAxis_X1, FrequencyScale_TransformFwd,
                               FrequencyScale_TransformInv, &m_spectrumFrequencyScale);

        ImPlot::SetupAxis(ImAxis_Y1, "Intensity [dB]", ImPlotAxisFlags_None);
        ImPlot::SetupAxisLimitsConstraints(ImAxis_Y1, -150, 50);
        ImPlot::SetupAxisLimits(ImAxis_Y1, -90, 20);

        ImPlot::SetupLegend(ImPlotLocation_NorthEast);

        setupPlotFrequencyTicks();

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

    m_sourceSpectrum.update();
    m_formantSpectrum.update();
    m_formantGenerator.updateSpectrumIfNeeded();
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

        Scalar value = param.value();
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

    snprintf(fieldLabel, bufferLength, "f%s:", subscriptK[k]);
    snprintf(fieldId, bufferLength, "##param_F%d", (k + 1));

    Scalar freq = m_formantGenerator.frequency(k);
    if (ScrollableDrag(fieldLabel, fLabelW, fieldId, 15 * em(), &freq, 200.0, 6000.0,
                       "%g Hz")) {
        m_formantGenerator.setFrequency(k, freq);
    }

    ImGui::SameLine();

    snprintf(fieldLabel, bufferLength, "b%s:", subscriptK[k]);
    snprintf(fieldId, bufferLength, "##param_B%d", (k + 1));

    Scalar band = m_formantGenerator.bandwidth(k);
    if (ScrollableDrag(fieldLabel, bLabelW, fieldId, 7.5 * em(), &band, 10.0, 600.0,
                       "%g Hz")) {
        m_formantGenerator.setBandwidth(k, band);
    }
}

bool SourceModelApp::ScrollableDrag(const char* fieldLabel, const float labelW,
                                    const char* fieldId, const float fieldW,
                                    Scalar* value, Scalar min, Scalar max,
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
    if (ImGui::DragFloatOrDouble(fieldId, value, std::pow(10.0_f, Scalar(precision - 1)),
                                 min, max, format)) {
        ImGui::EndGroup();
        return true;
    }
    ImGui::EndGroup();

    ImGuiIO& io = ImGui::GetIO();
    if (ImGui::IsItemHovered() && io.MouseWheel != 0) {
        *value =
            std::clamp(*value + 2 * powf(10.0f, precision - 1) * io.MouseWheel, min, max);
        return true;
    }
    return false;
}

void SourceModelApp::updateDownscaledPlot(const int count, const int start,
                                          const int end) {
    const int     N = m_glottalFlow.sampleCount();
    const Scalar* t = m_glottalFlow.t();

    const Scalar* g = m_glottalFlow.g();
    const auto&   gMin = m_glottalFlow.gMin();
    const auto&   gMax = m_glottalFlow.gMax();
    const Scalar  gAmp = m_glottalFlow.gAmplitude();

    const Scalar* dg = m_glottalFlow.dg();
    const auto&   dgMin = m_glottalFlow.dgMin();
    const auto&   dgMax = m_glottalFlow.dgMax();
    const Scalar  dgAmp = m_glottalFlow.dgAmplitude();

    m_downsampledCount = count;
    m_downsampledStart = start;
    m_downsampledEnd = end;

    m_gDownsampled = downsampleLTTB(t, g, N, count, start, end);
    m_dgDownsampled = downsampleLTTB(t, dg, N, count, start, end);

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

    if (m_doNormalizeFlowPlot) {
        for (auto& [x, y] : m_gDownsampled) {
            y /= gAmp;
        }
        for (auto& [x, y] : m_dgDownsampled) {
            y /= dgAmp;
        }
    }
}

void SourceModelApp::setupPlotFrequencyTicks() {
    ImPlotStyle& pStyle = ImPlot::GetStyle();
    ImPlotPlot*  plot = ImPlot::GetCurrentPlot();
    auto&        axis = plot->Axes[ImAxis_X1];

    const float pixelMin = plot->PlotRect.Min.x;
    const float pixelMax = plot->PlotRect.Max.x;

    // Make the tick values.

    m_majorTicks.clear();
    m_minorTicks.clear();
    m_minorMinorTicks.clear();

    if (m_spectrumFrequencyScale == FrequencyScale_Linear) {
        setupPlotFrequencyTicksLinear(axis);
    } else {
        setupPlotFrequencyTicksNonlinear(axis);
    }

    // Make the tick labels.

    m_majorTickLabels.clear();
    m_minorTickLabels.clear();
    m_minorMinorTickLabels.clear();
    m_tickBits.resize(
        (size_t)std::ceil(std::max(ImGui::GetContentRegionMaxAbs().x, pixelMax)));
    m_tickBits.reset();

    constexpr size_t labelSz = 64;
    char             labelTmp[labelSz];

    for (double value : m_majorTicks) {
        snprintf(labelTmp, labelSz, "%g", value);

        if (canInsertLabel(value, labelTmp, axis, pixelMin, pixelMax)) {
            m_majorTickLabels.push_back(labelTmp);
        } else {
            m_majorTickLabels.push_back("");
        }
    }

    for (double value : m_minorTicks) {
        snprintf(labelTmp, labelSz, "%g", value);

        if (canInsertLabel(value, labelTmp, axis, pixelMin, pixelMax)) {
            m_minorTickLabels.push_back(labelTmp);
        } else {
            m_minorTickLabels.push_back("");
        }
    }

    for (double value : m_minorMinorTicks) {
        snprintf(labelTmp, labelSz, "%g", value);

        if (canInsertLabel(value, labelTmp, axis, pixelMin, pixelMax)) {
            m_minorMinorTickLabels.push_back(labelTmp);
        } else {
            m_minorMinorTickLabels.push_back("");
        }
    }

    // Actually add the ticks to the data

    axis.ShowDefaultTicks = false;

    for (int i = 0; i < m_majorTicks.size(); ++i) {
        const bool  showLabel = (!m_majorTickLabels[i].empty());
        const char* label = m_majorTickLabels[i].c_str();
        axis.Ticker.AddTick(m_majorTicks[i], true, 0, showLabel, label);
    }

    for (int i = 0; i < m_minorTicks.size(); ++i) {
        const bool  showLabel = (!m_minorTickLabels[i].empty());
        const char* label = m_minorTickLabels[i].c_str();
        axis.Ticker.AddTick(m_minorTicks[i], false, 0, showLabel, label);
    }

    for (int i = 0; i < m_minorMinorTicks.size(); ++i) {
        const bool  showLabel = (!m_minorMinorTickLabels[i].empty());
        const char* label = m_minorMinorTickLabels[i].c_str();
        axis.Ticker.AddTick(m_minorMinorTicks[i], false, 0, showLabel, label);
    }
}

void SourceModelApp::setupPlotFrequencyTicksLinear(const ImPlotAxis& axis) {
    const double aMin = axis.Range.Min;
    const double aMax = axis.Range.Max;

    const double upp = 1.0 / axis.ScaleToPixel;

    // Calculate minor / major step.

    // 48 pixels between each tick.
    const double units = 48 * upp;
    double       minor, major;

    double d = 1e-6;
    bool   found = false;
    while (!found) {
        if (units < d) {
            minor = d;
            major = d * 5.0;
            found = true;
        } else {
            d *= 5.0;
            if (units < d) {
                minor = d;
                major = d * 2.0;
                found = true;
            } else {
                d *= 2.0;
            }
        }
    }
    if (!found) {
        minor = d;
        major = d * 2.0;
    }

    // Calculate ticks.

    double sg = upp > 0.0 ? 1.0 : -1.0;

    for (int jj = 0; jj < 2; ++jj) {
        const double denom = (jj == 0 ? major : minor);
        auto&        outputs = (jj == 0 ? m_majorTicks : m_minorTicks);
        int          ii = -1, j = 0;

        d = aMin - upp / 2;

        // using ints doesn't work, as this will overflow and be negative at high zoom.
        double step = std::floor(sg * d / denom);
        while (d <= aMax) {
            d += upp;

            if (std::floor(sg * d / denom) > step) {
                step = std::floor(sg * d / denom);
                outputs.push_back(sg * step * denom);
            }
        }
    }
}

void SourceModelApp::setupPlotFrequencyTicksNonlinear(const ImPlotAxis& axis) {
    const double aMin = axis.Range.Min;
    const double aMax = axis.Range.Max;

    const double loLog = std::log10(aMin);
    const double hiLog = std::log10(aMax) + 1;
    const int    loDecade = (int)std::floor(loLog);

    double       val;
    const double startDecade = std::pow(10, double(loDecade));

    const double delta(hiLog - loLog);
    const double step(delta >= 0 ? 10 : 0.1);
    const double rMin = std::min(aMin, aMax);
    const double rMax = std::max(aMin, aMax);

    // Major ticks are the decades
    double decade(startDecade);
    double steps(std::abs(delta));

    for (int i = 0; i <= steps; ++i) {
        val = decade;
        if (val >= rMin && val < rMax) {
            m_majorTicks.push_back(val);
        }
        decade *= step;
    }

    // Minor ticks are multiples of decades
    decade = startDecade;
    int start, end, mstep;
    if (delta > 0) {
        start = 2;
        end = 10;
        mstep = 1;
    } else {
        start = 9;
        end = 1;
        mstep = -1;
    }
    steps += 1;

    for (int i = 0; i <= (int)steps; ++i) {
        for (int j = start; j != end; j += mstep) {
            val = decade * j;
            if (val >= rMin && val < rMax) {
                m_minorTicks.push_back(val);
            }
        }
        decade *= step;
    }

    // MinorMinor ticks are multiples of decades
    decade = startDecade;
    if (delta > 0) {
        start = 10;
        end = 100;
        mstep = 1;
    } else {
        start = 100;
        end = 10;
        mstep = -1;
    }
    steps += 1;

    for (int i = 0; i <= (int)steps; ++i) {
        for (int f = start; f != end; f += mstep) {
            if (f == 50) continue;
            if ((int)(f / 10) != f / 10.0) {
                val = (decade * f) / 10;
                if (val >= rMin && val < rMax) {
                    m_minorMinorTicks.push_back(val);
                }
            }
        }
        decade *= step;
    }
}

bool SourceModelApp::canInsertLabel(double value, const char* label,
                                    const ImPlotAxis& axis, const float pixelMin,
                                    const float pixelMax) {
    // Position of the tick corresponding to that value.
    const float pixelPos = axis.PlotToPixels(value);

    // Calculate the size that the label would take.
    const ImVec2 labelSize = ImGui::CalcTextSize(label);
    const int    startX = (int)std::floor(pixelPos - 0.5f * labelSize.x);
    const int    endX = (int)std::ceil(pixelPos + 0.5f * labelSize.x);
    const int    N = endX - startX + 1;

    // Don't show label if the label clips off the end.
    if (startX <= 0 || endX >= pixelMax - 1) {
        return false;
    }

    // Check if there is already a label.
    for (int x = startX; x <= endX; ++x) {
        if (m_tickBits.test(x)) {
            return false;
        }
    }

    // If there is no label there already, set the bits and return true.
    m_tickBits.set(startX, N, true);

    return true;
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
    m_formantGenerator.spectrum().setSampleRate(m_audioOutput.sampleRate());
    m_audioOutput.setDevice(deviceInfo);
}

void SourceModelApp::audioErrorCallback(RtAudioErrorType   type,
                                        const std::string& errorText) {
    showMessages("RtAudio: " + errorText);
}
#endif