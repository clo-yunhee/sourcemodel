include(FetchContent)

# GLFW
if(CMAKE_SYSTEM_NAME STREQUAL "Emscripten")
    # Use Emscripten GLFW flag
else()
    FetchContent_Declare(
        glfw
        GIT_REPOSITORY https://github.com/glfw/glfw.git
        GIT_TAG        7482de6071d21db77a7236155da44c172a7f6c9e # 3.3.8
    )
    set(GLFW_BUILD_EXAMPLES OFF CACHE INTERNAL "")
    set(GLFW_BUILD_TESTS OFF CACHE INTERNAL "")
    set(GLFW_BUILD_DOCS OFF CACHE INTERNAL "")
    set(GLFW_INSTALL OFF CACHE INTERNAL "")
    FetchContent_MakeAvailable(glfw)
    add_subdirectory(glad)
endif()

# FreeType
FetchContent_Declare(
    freetype
    URL      https://download.savannah.gnu.org/releases/freetype/freetype-2.12.1.tar.gz
    URL_HASH SHA256=EFE71FD4B8246F1B0B1B9BFCA13CFFF1C9AD85930340C27DF469733BBB620938
)
foreach(_freetype_option "SKIP_INSTALL_HEADERS" "SKIP_INSTALL_LIBRARIES"
            "CMAKE_DISABLE_FIND_PACKAGE_PNG" "CMAKE_DISABLE_FIND_PACKAGE_HarfBuzz"
            "CMAKE_DISABLE_FIND_PACKAGE_BZip2" "CMAKE_DISABLE_FIND_PACKAGE_ZLIB"
            "CMAKE_DISABLE_FIND_PACKAGE_BROTLI")
    set(${_freetype_option} TRUE CACHE INTERNAL "")
endforeach()

if(NOT freetype_POPULATED)
    FetchContent_Populate(freetype)

    file(GLOB _freetype_patch "${CMAKE_CURRENT_SOURCE_DIR}/freetype/*")
    file(COPY ${_freetype_patch} DESTINATION ${freetype_SOURCE_DIR})

    add_subdirectory(${freetype_SOURCE_DIR} ${freetype_BINARY_DIR})
endif()

# RtAudio
if(NOT CMAKE_SYSTEM_NAME STREQUAL "Emscripten")
    add_subdirectory(rtaudio)
endif()

# RtAudio

# fftw3
FetchContent_Declare(
    fftw3
    URL      https://fftw.org/fftw-3.3.10.tar.gz
    URL_HASH SHA256=56C932549852CDDCFAFDAB3820B0200C7742675BE92179E59E6215B340E26467
)
set(BUILD_SHARED_LIBS OFF CACHE INTERNAL "")
set(BUILD_TESTS OFF CACHE INTERNAL "")
set(ENABLE_SSE ON CACHE INTERNAL "")
set(ENABLE_FLOAT OFF CACHE INTERNAL "")
if(CMAKE_CXX_COMPILER_ID STREQUAL MSVC OR CMAKE_CXX_SIMULATE_ID STREQUAL MSVC)
    set(LIBM_LIBRARY OFF CACHE INTERNAL "")
endif()

FetchContent_GetProperties(fftw3)
if(NOT fftw3_POPULATED)
    FetchContent_Populate(fftw3)

    file(GLOB _fftw3_patch "${CMAKE_CURRENT_SOURCE_DIR}/fftw3/*")
    file(COPY ${_fftw3_patch} DESTINATION ${fftw3_SOURCE_DIR})

    add_subdirectory(${fftw3_SOURCE_DIR} ${fftw3_BINARY_DIR})

    target_include_directories(fftw3 INTERFACE $<BUILD_INTERFACE:${fftw3_SOURCE_DIR}/api>)
    if(WIN32)
        target_compile_definitions(fftw3 PRIVATE HAVE_ALIGNED_MALLOC _CRT_SECURE_NO_WARNINGS)
    endif()
endif()

# imgui and implot
add_subdirectory(imgui EXCLUDE_FROM_ALL)
add_subdirectory(implot)

# speex
add_subdirectory(speexdsp)

# lsignal  (https://github.com/cpp11nullptr/lsignal)
FetchContent_Declare(
    sigslot
    GIT_REPOSITORY https://github.com/palacaze/sigslot
    GIT_TAG        e41c6e62edfa67b6402b17a627dca698a14f47a8 # v1.2.1
)
FetchContent_MakeAvailable(sigslot)

# Boost
FetchContent_Declare(
    Boost
    URL      https://github.com/boostorg/boost/releases/download/boost-1.81.0/boost-1.81.0.tar.gz
    URL_HASH SHA256=121DA556B718FD7BD700B5F2E734F8004F1CFA78B7D30145471C526BA75A151C
)
set(BOOST_INCLUDE_LIBRARIES "circular_buffer;math;multiprecision" CACHE INTERNAL "")
FetchContent_MakeAvailable(Boost)

# NFParam (AudioParam-like library)
FetchContent_Declare(
    NFParam
    GIT_REPOSITORY https://github.com/clo-yunhee/NFParam
    GIT_TAG        ae920f0a1ed5c3a9c2cfa76cae48b2addc5c855d
)
set(BUILD_TESTING OFF CACHE INTERNAL "")
FetchContent_MakeAvailable(NFParam)