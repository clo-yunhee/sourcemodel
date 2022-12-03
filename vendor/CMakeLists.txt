include(FetchContent)

# GLFW
if(EMSCRIPTEN)

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
FetchContent_MakeAvailable(freetype)

# portaudio
FetchContent_Declare(
    portaudio
    GIT_REPOSITORY https://github.com/PortAudio/portaudio.git
    GIT_TAG        af42647e3804f6a2472a00d943aaeedd641f7966
)
set(PA_BUILD_STATIC ON CACHE INTERNAL "")
set(PA_BUILD_SHARED OFF CACHE INTERNAL "")
set(PA_ENABLE_DEBUG_OUTPUT OFF CACHE INTERNAL "")
FetchContent_MakeAvailable(portaudio)

# fftw3
FetchContent_Declare(
    fftw3
    URL      https://fftw.org/fftw-3.3.10.tar.gz
    URL_HASH SHA256=56C932549852CDDCFAFDAB3820B0200C7742675BE92179E59E6215B340E26467
)
set(BUILD_SHARED_LIBS OFF CACHE INTERNAL "")
set(BUILD_TESTS OFF CACHE INTERNAL "")
set(ENABLE_SSE ON CACHE INTERNAL "")
set(ENABLE_FLOAT ON CACHE INTERNAL "")
if(CMAKE_CXX_COMPILER_ID STREQUAL MSVC OR CMAKE_CXX_SIMULATE_ID STREQUAL MSVC)
    set(LIBM_LIBRARY OFF CACHE INTERNAL "")
endif()

FetchContent_GetProperties(fftw3)
if(NOT fftw3_POPULATED)
    FetchContent_Populate(fftw3)

    file(GLOB _fftw3_patch "${CMAKE_CURRENT_SOURCE_DIR}/fftw3/*")
    file(COPY ${_fftw3_patch} DESTINATION ${fftw3_SOURCE_DIR})

    add_subdirectory(${fftw3_SOURCE_DIR} ${fftw3_BINARY_DIR})

    target_include_directories(fftw3f INTERFACE $<BUILD_INTERFACE:${fftw3_SOURCE_DIR}/api>)
    if(WIN32)
        target_compile_definitions(fftw3f PRIVATE HAVE_ALIGNED_MALLOC)
    endif()
endif()

# imgui and implot
add_subdirectory(imgui EXCLUDE_FROM_ALL)
add_subdirectory(implot)

# speex
add_subdirectory(speexdsp)