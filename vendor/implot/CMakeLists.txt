FetchContent_Declare(
    implot
    GIT_REPOSITORY https://github.com/epezent/implot.git
    GIT_TAG        d87512353495e7760e7fda7566a05beef7627d8f # latest
)

if(NOT implot_POPULATED)
    FetchContent_Populate(implot)

    add_library(ImPlot STATIC)

    target_sources(ImPlot PRIVATE
        ${implot_SOURCE_DIR}/implot_internal.h
        ${implot_SOURCE_DIR}/implot_items.cpp
        ${implot_SOURCE_DIR}/implot.cpp
        ${implot_SOURCE_DIR}/implot.h)

    target_include_directories(ImPlot PUBLIC
        $<BUILD_INTERFACE:${implot_SOURCE_DIR}>)

    target_link_libraries(ImPlot PUBLIC ImGui::Core)
endif()

