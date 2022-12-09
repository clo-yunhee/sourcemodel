set(_target SourceModel)

add_executable(${_target}
    math/LTTB.cpp
    math/LTTB.h
    math/utils.h
    models/KLGLOTT88.cpp
    models/KLGLOTT88.h
    models/LF.cpp
    models/LF.h
    models/RosenbergC.cpp
    models/RosenbergC.h
    models/RPlusPlus.cpp
    models/RPlusPlus.h
    Application.cpp
    Application.h
    GlottalFlow.cpp
    GlottalFlow.h
    GlottalFlowModel.h
    GlottalFlowParameter.cpp
    GlottalFlowParameter.h
    GlottalFlowParameters.h
    imgui_user.cpp
    imgui_user.h
    main.cpp
    SourceModelApp.cpp
    SourceModelApp.h
)

add_embedded_file(${_target} "fonts/roboto-regular.woff"
                    font_roboto.h
                    gFontRoboto)

add_embedded_file(${_target} "fonts/vollkorn-regular.woff"
                    font_vollkorn.h
                    gFontVollkorn)

target_include_directories(${_target} PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})

target_link_libraries(${_target}
    PRIVATE PortAudio
            ImGui::Core
            ImGui::ImplGlfw
            ImGui::ImplOpenGL3
            ImGui::FreeType freetype
            ImPlot
            fftw3f speex_resampler
            Pal::Sigslot
            Boost::math
)

set_property(TARGET ${_target} PROPERTY CXX_STANDARD 20)
set_property(TARGET ${_target} PROPERTY CXX_STANDARD_REQUIRED TRUE)

target_compile_definitions(${_target} PRIVATE "IMGUI_INCLUDE_IMGUI_USER_H")

if(CMAKE_BUILD_TYPE STREQUAL "Release")
    set_property(TARGET ${_target}
                 PROPERTY INTERPROCEDURAL_OPTIMIZATION TRUE)
endif()

if(CMAKE_BUILD_TYPE STREQUAL "Debug" OR CMAKE_BUILD_TYPE STREQUAL "RelWithDebInfo")
    target_compile_definitions(${_target} PRIVATE "SOURCEMODEL_DEBUG")
endif()

if(CMAKE_SYSTEM_NAME STREQUAL "Windows")
    target_compile_definitions(${_target} PRIVATE "IMGUI_ENABLE_WIN32_DEFAULT_IME_FUNCTIONS")
    target_compile_options(${_target} PRIVATE "/utf-8")

    # Only disable console in Release build
    if(CMAKE_BUILD_TYPE STREQUAL "Release")
        if(MSVC)
            target_link_options(${_target} PRIVATE "/subsystem:windows")
        else()
            target_link_options(${_target} PRIVATE "-mwindows")
        endif()
        target_compile_definitions(${_target} PRIVATE "WINMAIN")
    endif()

    target_link_libraries(${_target} PRIVATE "psapi")
    
    target_compile_definitions(${_target} PRIVATE "UNICODE" "_USE_MATH_DEFINES"
                                                 "_CRT_SECURE_NO_WARNINGS" "NOMINMAX")
endif()

if(CMAKE_SYSTEM_NAME STREQUAL "Emscripten")
    # Using shell.html for HTML output file.
    set_target_properties(${_target} PROPERTIES
        SUFFIX ".html"
        LINK_DEPENDS "${CMAKE_CURRENT_SOURCE_DIR}/shell.html")
    target_link_options(${_target} PRIVATE "SHELL:--shell-file ${CMAKE_CURRENT_SOURCE_DIR}/shell.html")

    # Disable Emscripten filesystem
    target_link_options(${_target} PRIVATE "SHELL:-s NO_FILESYSTEM=1")
    target_compile_definitions(${_target} PRIVATE "IMGUI_DISABLE_FILE_FUNCTIONS")

    # Other EMS flags
    target_link_options(${_target} PRIVATE "SHELL:-s ALLOW_MEMORY_GROWTH=1"
                                           "SHELL:-s NO_EXIT_RUNTIME=0")

    if(CMAKE_BUILD_TYPE STREQUAL "Release")
        target_compile_options(${_target} PRIVATE "-O2")
        target_link_options(${_target} PRIVATE "-O2")
    elseif(CMAKE_BUILD_TYPE STREQUAL "RelWithDebInfo")
        target_compile_options(${_target} PRIVATE "-O1" "-g2")
        target_link_options(${_target} PRIVATE "-O1" "-g2")
    elseif(CMAKE_BUILD_TYPE STREQUAL "Debug")
        target_compile_options(${_target} PRIVATE "-O1" "-g")
        target_link_options(${_target} PRIVATE "-O1" "-g")
    endif()
endif()

if(USE_ASAN)
    target_compile_options(${_target} PRIVATE "-fsanitize=address")
    target_link_options(${_target} PRIVATE "-fsanitize=address")
endif()

if(USE_USAN)
    target_compile_options(${_target} PRIVATE "-fsanitize=undefined")
    target_link_options(${_target} PRIVATE "-fsanitize=undefined")
endif()
