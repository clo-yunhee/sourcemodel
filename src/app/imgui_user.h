#ifndef SOURCEMODEL__IMGUI_USER_H
#define SOURCEMODEL__IMGUI_USER_H

#include <type_traits>

namespace ImGui {

void BeginGroupPanel(const char* name, const ImVec2& size = ImVec2(0.0f, 0.0f));
void EndGroupPanel();

bool SliderDouble(const char* label, double* v, double v_min, double v_max,
                  const char* format = "%.3f", ImGuiSliderFlags flags = 0);

bool DragDouble(const char* label, double* v, double v_speed, double v_min, double v_max,
                const char* format = "%.3f", ImGuiSliderFlags flags = 0);

template <typename T, typename = std::enable_if_t<std::is_same_v<float, T> ||
                                                  std::is_same_v<double, T>>>
bool SliderFloatOrDouble(const char* label, T* v, T v_min, T v_max,
                         const char* format = "%.3f", ImGuiSliderFlags flags = 0) {
    if constexpr (std::is_same_v<T, float>) {
        return SliderFloat(label, v, v_min, v_max, format, flags);
    } else if constexpr (std::is_same_v<T, double>) {
        return SliderDouble(label, v, v_min, v_max, format, flags);
    }
}

template <typename T, typename = std::enable_if_t<std::is_same_v<float, T> ||
                                                  std::is_same_v<double, T>>>
bool DragFloatOrDouble(const char* label, T* v, T v_speed, T v_min, T v_max,
                       const char* format = "%.3f", ImGuiSliderFlags flags = 0) {
    if constexpr (std::is_same_v<T, float>) {
        return DragFloat(label, v, v_speed, v_min, v_max, format, flags);
    } else if constexpr (std::is_same_v<T, double>) {
        return DragDouble(label, v, v_speed, v_min, v_max, format, flags);
    }
}

}  // namespace ImGui

#endif  // SOURCEMODEL__IMGUI_USER_H