set(_target SourceModel)

add_executable(${_target}
    audio/AudioTime.h
    audio/BufferedGenerator.cpp
    audio/BufferedGenerator.h
    audio/GainReductionComputer.cpp
    audio/GainReductionComputer.h
    audio/LookAheadGainReduction.cpp
    audio/LookAheadGainReduction.h
    math/filters/Butterworth.cpp
    math/filters/Butterworth.h
    math/filters/SOSFilter.cpp
    math/filters/SOSFilter.h
    math/filters/SVFBiquad.cpp
    math/filters/SVFBiquad.h
    math/filters/SVFPiece.cpp
    math/filters/SVFPiece.h
    math/filters/zpk2sos.cpp
    math/DTFT.h
    math/FrequencyScale.cpp
    math/FrequencyScale.h
    math/LTTB.cpp
    math/LTTB.h
    math/utils.h
    models/KLGLOTT88.cpp
    models/KLGLOTT88.h
    models/LF_precomputed_Rd_double.inc.h
    models/LF_precomputed_Rd_float.inc.h
    models/LF.cpp
    models/LF.h
    models/RosenbergC.cpp
    models/RosenbergC.h
    models/RPlusPlus.cpp
    models/RPlusPlus.h
    Application.cpp
    Application.h
    CachedGlottalFlowModel.cpp
    CachedGlottalFlowModel.h
    FilterSpectrum.cpp
    FilterSpectrum.h
    FormantGenerator.cpp
    FormantGenerator.h
    GeneratorSpectrum.cpp
    GeneratorSpectrum.h
    GlottalFlow.cpp
    GlottalFlow.h
    GlottalFlowModel.h
    GlottalFlowParameter.cpp
    GlottalFlowParameter.h
    GlottalFlowParameters.h
    imgui_user.cpp
    imgui_user.h
    main.cpp
    OneFormantFilter.cpp
    OneFormantFilter.h
    SourceGenerator.cpp
    SourceGenerator.h
    SourceModelApp.cpp
    SourceModelApp.h
)

add_embedded_font(${_target} "fonts/roboto-regular.ttf"
                    font_roboto.h
                    gFontRoboto
                    "0020-007E,00A0-00FF,03B1,2080-2085,2090,2098")

add_embedded_font(${_target} "fonts/vollkorn-bold.ttf"
                    font_vollkorn.h
                    gFontVollkorn
                    "2208")

add_embedded_font(${_target} "fonts/fa-solid.ttf"
                    font_faSolid.h
                    gFontFaSolid
                    "F026-F028,F04B-F04C,F6A8-F6A9")

target_include_directories(${_target} PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})

target_link_libraries(${_target}
    PRIVATE ImGui::Core
            ImGui::ImplGlfw
            ImGui::ImplOpenGL3
            ImGui::FreeType freetype
            ImPlot
            speex_resampler
            Pal::Sigslot
            Boost::math
            Boost::circular_buffer
            Boost::dynamic_bitset
            Boost::lockfree
            NFParam
)

set_property(TARGET ${_target} PROPERTY CXX_STANDARD 20)
set_property(TARGET ${_target} PROPERTY CXX_STANDARD_REQUIRED TRUE)

# Enable LTO.
if(CMAKE_BUILD_TYPE STREQUAL "Release")
    set_property(TARGET ${_target}
                 PROPERTY INTERPROCEDURAL_OPTIMIZATION TRUE)
endif()

if(CMAKE_BUILD_TYPE STREQUAL "Debug" OR CMAKE_BUILD_TYPE STREQUAL "RelWithDebInfo")
    target_compile_definitions(${_target} PRIVATE "SOURCEMODEL_DEBUG")
endif()

target_compile_definitions(${_target} PRIVATE "IMGUI_INCLUDE_IMGUI_USER_H")

# Platform specific flags for Windows.
if(CMAKE_SYSTEM_NAME STREQUAL "Windows")
    target_compile_definitions(${_target} PRIVATE "IMGUI_ENABLE_WIN32_DEFAULT_IME_FUNCTIONS")

    if(MSVC)
        target_compile_options(${_target} PRIVATE "/utf-8")
    endif()

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
        target_compile_options(${_target} PRIVATE "-O3")
        target_link_options(${_target} PRIVATE "-O3" "--closure 2")
    elseif(CMAKE_BUILD_TYPE STREQUAL "RelWithDebInfo")
        target_compile_options(${_target} PRIVATE "-O1" "-g2")
        target_link_options(${_target} PRIVATE "-O1" "-g2")
    elseif(CMAKE_BUILD_TYPE STREQUAL "Debug")
        target_compile_options(${_target} PRIVATE "-O1" "-g")
        target_link_options(${_target} PRIVATE "-O1" "-g" "-gsource-map" "--source-map-base=/")
    endif()
endif()

# On Emscripten use SDL2 audio, else use PortAudio
if(CMAKE_SYSTEM_NAME STREQUAL "Emscripten")
    target_compile_definitions(${_target} PRIVATE "USING_WEBAUDIO")
    target_link_options(${_target} PRIVATE "SHELL:-s USE_SDL=2")
    target_sources(${_target} PRIVATE
        audio/webaudio/AudioOutput.cpp
        audio/webaudio/AudioOutput.h
    )
else()
    target_compile_definitions(${_target} PRIVATE "USING_RTAUDIO")
    target_link_libraries(${_target} PRIVATE RtAudio::RtAudio)
    target_sources(${_target} PRIVATE
        audio/rtaudio/AudioDevices.cpp
        audio/rtaudio/AudioDevices.h
        audio/rtaudio/AudioOutput.cpp
        audio/rtaudio/AudioOutput.h
    )
endif()

# On Emscripten use single-precision floats
if(CMAKE_SYSTEM_NAME STREQUAL "Emscripten")
    target_compile_definitions(${_target} PRIVATE "USING_SINGLE_FLOAT")
    target_link_libraries(${_target} PRIVATE fftw3f) 
else()
    # fftw3 or fftw3f depending on 64- or 32-bit.
    if(CMAKE_SIZEOF_VOID_P EQUAL 8)
        target_compile_definitions(${_target} PRIVATE "USING_DOUBLE_FLOAT")
        target_link_libraries(${_target} PRIVATE fftw3)
    elseif(CMAKE_SIZEOF_VOID_P EQUAL 4)
        target_compile_definitions(${_target} PRIVATE "USING_SINGLE_FLOAT")
        target_link_libraries(${_target} PRIVATE fftw3f) 
    endif()
endif()

if(USE_ASAN)
    if(MSVC)
        target_compile_definitions(${_target} PRIVATE
            "_DISABLE_VECTOR_ANNOTATION"
            "_DISABLE_STRING_ANNOTATION"
        )
        target_compile_options(${_target} PRIVATE "/fsanitize=address" "/fsanitize-address-use-after-return")
        target_link_options(${_target} PRIVATE "/INCREMENTAL:NO")
    else()
        target_compile_options(${_target} PRIVATE "-fsanitize=address")
        target_link_options(${_target} PRIVATE "-fsanitize=address")
    endif()
endif()

if(USE_USAN AND NOT MSVC)
    target_compile_options(${_target} PRIVATE "-fsanitize=undefined")
    target_link_options(${_target} PRIVATE "-fsanitize=undefined")
endif()

# Set optimization flags for Release
if(CMAKE_BUILD_TYPE STREQUAL "Release")
    if(CMAKE_CXX_COMPILER_ID STREQUAL "MSVC")
        target_compile_options(${_target} PRIVATE /fp:fast /arch:SSE /arch:SSE2 /arch:AVX /arch:AVX2 /O2 /GL)
        target_link_options(${_target}    PRIVATE /fp:fast /arch:SSE /arch:SSE2 /arch:AVX /arch:AVX2 /O2 /GL)
    elseif(CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
        target_compile_options(${_target} PRIVATE -msse -msse2 -mavx -mavx2 -O3)
        target_link_options(${_target}    PRIVATE -msse -msse2 -mavx -mavx2 -O3)
    endif()
endif()

# Precompute Rd parameters custom target
add_subdirectory(models/precompute)