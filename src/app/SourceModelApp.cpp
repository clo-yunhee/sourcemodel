#include "SourceModelApp.h"

#include <implot.h>
#include <implot_internal.h>

#include <algorithm>
#include <cmath>
#include <iostream>

#include "math/LTTB.h"
#include "math/utils.h"

SourceModelApp::SourceModelApp(const int initialWidth, const int initialHeight)
    : Application("SourceModel", initialWidth, initialHeight) {
    ImPlot::CreateContext();

    m_glottalFlow.setSampleCount(1024);
    m_glottalFlow.setModelType(GlottalFlowModel_LF);  // Default model to LF.
}

SourceModelApp::~SourceModelApp() { ImPlot::DestroyContext(); }

void SourceModelApp::setupThemeColors(ImGuiStyle& style) {
    if (isDarkTheme()) {
        ImGui::StyleColorsDark(&style);
        style.Colors[ImGuiCol_Text] = ImVec4(0.98f, 0.98f, 0.98f, 1.0f);
        ImPlot::StyleColorsDark();
    } else {
        ImGui::StyleColorsLight(&style);
        style.Colors[ImGuiCol_Text] = ImVec4(0.14f, 0.14f, 0.14f, 1.0f);
        ImPlot::StyleColorsLight();
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

    int type = (int)m_glottalFlow.modelType();
    if (ImGui::Combo("Type", &type, GlottalFlowModel_NAMES)) {
        m_glottalFlow.setModelType((GlottalFlowModelType)type);
    }

    renderParameterControl(m_glottalFlow.parameters().Oq, "Oq", "Oq", 2);

    renderParameterControl(m_glottalFlow.parameters().am, "am", "\u03B1\u2098", 3);

    renderParameterControl(m_glottalFlow.parameters().Qa, "Qa", "Q\u2090", 3);

    ImGui::EndGroupPanel();  // Glottal flow parameters

    static bool normalize = true;
    ImGui::Checkbox("Normalize", &normalize);

    if (ImPlot::BeginPlot("##gfm_timeplot")) {
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

        const double gScale = (normalize ? 1.0 / m_glottalFlow.gAmplitude() : 1);
        const double dgScale = (normalize ? 1.0 / m_glottalFlow.dgAmplitude() : 1);

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

        ImPlot::PlotLineG("g(t)", plotGetterLTTB, g.Data, g.size());
        ImPlot::PlotLineG("dg(t)", plotGetterLTTB, dg.Data, dg.size());

        ImPlot::PopStyleVar(ImPlotStyleVar_LineWeight);

        ImPlot::EndPlot();
    }

    ImGui::EndChild();  // ChildL
}

void SourceModelApp::renderOther() {}

void SourceModelApp::renderParameterControl(GlottalFlowParameter& param, const char* name,
                                            const char* displayName, int precision) {
    if (param.isFixed()) {
        ImGui::AlignTextToFramePadding();
        ImGui::Text("%s = %g", displayName, param.value());
    } else {
        static constexpr int bufferLength = 32;
        static char          label[bufferLength];

        snprintf(label, bufferLength, "##gfm_%s", name);

        ImGui::BeginGroup();
        ImGui::AlignTextToFramePadding();
        ImGui::Text("%s \u2208 [%g, %g]", displayName, param.min(), param.max());
        ImGui::SameLine();
        double value = param.value();
        if (ImGui::DragDouble(label, &value, pow(10, -precision), param.min(),
                              param.max(), "%g")) {
            param.setValue(value);
        }
        ImGui::EndGroup();

        ImGuiIO& io = ImGui::GetIO();
        if (ImGui::IsItemHovered() && io.MouseWheel != 0) {
            value = std::clamp(value + 2 * pow(10, -precision) * io.MouseWheel,
                               param.min(), param.max());
            if (!fuzzyEquals(param.value(), value)) {
                param.setValue(value);
            }
        }
    }
}