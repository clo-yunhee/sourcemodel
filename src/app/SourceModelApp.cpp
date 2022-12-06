#include "SourceModelApp.h"

#include <implot.h>

#include <algorithm>
#include <cmath>
#include <iostream>

#include "math_utils.h"

SourceModelApp::SourceModelApp(const int initialWidth, const int initialHeight)
    : Application("SourceModel", initialWidth, initialHeight) {
    ImPlot::CreateContext();

    m_glottalFlow.setSampleCount(500);
    m_glottalFlow.setModel<LFModel>();  // Default model to LF.
}

SourceModelApp::~SourceModelApp() { ImPlot::DestroyContext(); }

void SourceModelApp::setupThemeColors(ImGuiStyle& style) {
    if (isDarkTheme()) {
        ImGui::StyleColorsDark(&style);
        style.Colors[ImGuiCol_Text] = ImVec4(0.98f, 0.98f, 0.98f, 1.0f);
    } else {
        ImGui::StyleColorsLight(&style);
        style.Colors[ImGuiCol_Text] = ImVec4(0.14f, 0.14f, 0.14f, 1.0f);
    }
}

void SourceModelApp::setupThemeSizes(ImGuiStyle& style) { style.WindowRounding = 3.0f; }

void SourceModelApp::renderMain() {
    ImGuiIO& io = ImGui::GetIO();

    if (m_glottalFlow.isDirty()) {
        m_glottalFlow.updateSamples();
    }

    ImGui::BeginChild("ChildL", ImVec2(ImGui::GetWindowContentRegionWidth() * 0.5f, -1));
    ImGui::BeginGroupPanel("Glottal flow parameters");
    ImGui::Indent();
    ImGui::Text("E\u2080 = 1 ; T\u2080 = 1");
    ImGui::Unindent();
    ImGui::Spacing();

    renderParameterControl(m_glottalFlow.parameters().Oq, m_glottalFlow.parameters().Oq,
                           "Oq", "Oq", 2);

    renderParameterControl(m_glottalFlow.parameters().am, m_glottalFlow.parameters().am,
                           "am", "\u03B1\u2098", 2);

    renderParameterControl(m_glottalFlow.parameters().Qa, m_glottalFlow.parameters().Qa,
                           "Qa", "Q\u2090", 3);

    ImGui::EndGroupPanel();  // Glottal flow parameters

    if (ImPlot::BeginPlot("##gfm_timeplot")) {
        ImPlot::SetupAxes("Time", "SPL");
        ImPlot::SetupAxesLimits(0, 1, -1.1, 1.1);

        ImPlot::SetupLegend(ImPlotLocation_NorthEast);
        ImPlot::PlotLine("dg(t)", m_glottalFlow.t(), m_glottalFlow.dg(),
                         m_glottalFlow.sampleCount());
        ImPlot::PlotLine("g(t)", m_glottalFlow.t(), m_glottalFlow.g(),
                         m_glottalFlow.sampleCount());
        // ImPlot::PlotLine("g'(t)", t, g2, N);
        ImPlot::EndPlot();
    }

    ImGui::EndChild();  // ChildL
}

void SourceModelApp::renderOther() {}

void SourceModelApp::renderParameterControl(const GlottalFlowParameter& paramSrc,
                                            GlottalFlowParameter&       paramDst,
                                            const char* name, const char* displayName,
                                            const int precision) {
    static constexpr int bufferLength = 32;
    static char          floatFmtStr[bufferLength];
    static char          label[bufferLength];

    snprintf(floatFmtStr, bufferLength, "%%.%df", precision);
    snprintf(label, bufferLength, "##gfm_%s", name);

    // Get the precision for drag & scroll

    ImGui::BeginGroup();
    ImGui::AlignTextToFramePadding();
    ImGui::Text("%s \u2208 [%g, %g]", displayName, paramSrc.min(), paramSrc.max());
    ImGui::SameLine();
    double value = paramSrc.value();
    if (ImGui::DragDouble(label, &value, 0.001, paramSrc.min(), paramSrc.max(),
                          floatFmtStr)) {
        paramDst.setValue(value);
    }
    ImGui::EndGroup();

    ImGuiIO& io = ImGui::GetIO();
    if (ImGui::IsItemHovered() && io.MouseWheel != 0) {
        value = std::clamp(value + 0.02 * io.MouseWheel, paramSrc.min(), paramSrc.max());
        if (!fuzzyEquals(paramSrc.value(), value)) {
            paramDst.setValue(value);
        }
    }
}