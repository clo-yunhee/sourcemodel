FetchContent_Declare(
    implot
    GIT_REPOSITORY https://github.com/epezent/implot.git
    GIT_TAG        18758e237e8906a97ddf42de1e75793526f30ce9 # latest
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

