
FetchContent_Declare(
    fftw3
    URL      https://fftw.org/fftw-3.3.10.tar.gz
    URL_HASH SHA256=56C932549852CDDCFAFDAB3820B0200C7742675BE92179E59E6215B340E26467
)
set(BUILD_SHARED_LIBS OFF CACHE INTERNAL "")
set(BUILD_TESTS OFF CACHE INTERNAL "")
if(CMAKE_CXX_COMPILER_ID STREQUAL MSVC OR CMAKE_CXX_SIMULATE_ID STREQUAL MSVC)
    set(LIBM_LIBRARY OFF CACHE INTERNAL "")
endif()

FetchContent_GetProperties(fftw3)
if(NOT fftw3_POPULATED)
    FetchContent_Populate(fftw3)

    file(GLOB _fftw3_patch "${CMAKE_CURRENT_SOURCE_DIR}/kernel/*")
    file(COPY ${_fftw3_patch} DESTINATION ${fftw3_SOURCE_DIR}/kernel)

    file(GLOB _fftw3_srcs "${fftw3_SOURCE_DIR}/*")

    # Double
    file(COPY ${_fftw3_srcs} DESTINATION "${fftw3_SOURCE_DIR}_double")
    set(ENABLE_FLOAT OFF CACHE INTERNAL "")
    set(ENABLE_SSE2 ON CACHE INTERNAL "")
    add_subdirectory("${fftw3_SOURCE_DIR}_double" "${fftw3_BINARY_DIR}_double")
    target_include_directories(fftw3 INTERFACE $<BUILD_INTERFACE:${fftw3_SOURCE_DIR}_double/api>)
    if(WIN32)
        target_compile_definitions(fftw3 PRIVATE HAVE_ALIGNED_MALLOC _CRT_SECURE_NO_WARNINGS)
    endif()

    # Float
    file(COPY ${_fftw3_srcs} DESTINATION "${fftw3_SOURCE_DIR}_float")
    set(ENABLE_FLOAT ON CACHE INTERNAL "")
    set(ENABLE_SSE ON CACHE INTERNAL "")
    add_subdirectory("${fftw3_SOURCE_DIR}_float" "${fftw3_BINARY_DIR}_float")
    target_include_directories(fftw3f INTERFACE $<BUILD_INTERFACE:${fftw3_SOURCE_DIR}_float/api>)
    if(WIN32)
        target_compile_definitions(fftw3f PRIVATE HAVE_ALIGNED_MALLOC _CRT_SECURE_NO_WARNINGS)
    endif()

    target_include_directories(fftw3  INTERFACE $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>)
    target_include_directories(fftw3f INTERFACE $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>)
endif()