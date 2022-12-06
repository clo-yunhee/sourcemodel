#ifndef SOURCEMODEL__IMGUI_USER_H
#define SOURCEMODEL__IMGUI_USER_H

namespace ImGui {

void BeginGroupPanel(const char* name, const ImVec2& size = ImVec2(0.0f, 0.0f));
void EndGroupPanel();

bool SliderDouble(const char* label, double* v, double v_min, double v_max,
                  const char* format = "%.3f", ImGuiSliderFlags flags = 0);

bool DragDouble(const char* label, double* v, double v_speed = 1.0f, double v_min = 0.0f,
                double v_max = 0.0f, const char* format = "%.3f",
                ImGuiSliderFlags flags = 0);

}  // namespace ImGui

#endif  // SOURCEMODEL__IMGUI_USER_H