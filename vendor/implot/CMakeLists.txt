#FetchContent_Declare(
#    implot
#    GIT_REPOSITORY https://github.com/in-formant/implot.git
#    GIT_TAG        5bd1a570d33ca48c4f7d5279a2aa2d7208cbf99d # latest scales
#)

#[[FetchContent_Declare(
    implot
    GIT_REPOSITORY https://github.com/epezent/implot.git
    GIT_TAG        8879c99aef760379c85d5a7a0de7f70e7af3210c
)

if(NOT implot_POPULATED)
    FetchContent_Populate(implot)]]

if(TRUE)
    set(implot_SOURCE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/implot_dev)

    add_library(ImPlot STATIC)

    target_sources(ImPlot PRIVATE
        ${implot_SOURCE_DIR}/implot_internal.h
        ${implot_SOURCE_DIR}/implot_items.cpp
        ${implot_SOURCE_DIR}/implot.cpp
        ${implot_SOURCE_DIR}/implot.h)

    target_include_directories(ImPlot PUBLIC
        $<BUILD_INTERFACE:${implot_SOURCE_DIR}>)

    target_link_libraries(ImPlot PUBLIC ImGui::Core)

    if(USE_OPENGL3)
        target_sources(ImPlot PRIVATE
            ${implot_SOURCE_DIR}/backends/implot_backend.h
            ${implot_SOURCE_DIR}/backends/implot_impl_opengl3.cpp
            ${implot_SOURCE_DIR}/backends/implot_impl_opengl3.h)
        target_link_libraries(ImPlot PUBLIC glad)
        target_compile_definitions(ImPlot PUBLIC "IMPLOT_BACKEND_ENABLE_OPENGL3")
    endif()

    if(FALSE AND USE_VULKAN)
        target_sources(ImPlot PRIVATE
            ${implot_SOURCE_DIR}/backends/implot_backend.h
            ${implot_SOURCE_DIR}/backends/implot_impl_vulkan.cpp
            ${implot_SOURCE_DIR}/backends/vulkan/heatmap.frag.u32
            ${implot_SOURCE_DIR}/backends/vulkan/heatmap.vert.u32)
        target_link_libraries(ImPlot PUBLIC ImGui::ImplVulkan glm::glm)
        target_compile_definitions(ImPlot PUBLIC "IMPLOT_BACKEND_ENABLE_VULKAN")
    endif()
endif()

