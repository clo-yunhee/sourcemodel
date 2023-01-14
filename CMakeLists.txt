cmake_minimum_required(VERSION 3.13...3.22 FATAL_ERROR)

project(SourceModel LANGUAGES C CXX)
set(PROJECT_VERSION 1.0)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

if(NOT CMAKE_BUILD_TYPE)
    message(STATUS "CMAKE_BUILD_TYPE was not set: defaults to RelWithDebInfo")
    set(CMAKE_BUILD_TYPE RelWithDebInfo)
endif()

if(MSVC)
    set(CMAKE_MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>")
endif()

if(CMAKE_SYSTEM_NAME STREQUAL "Windows")
    add_definitions(-D_WIN32_WINNT=0x0601)
endif()

option(USE_ASAN "Build with AddressSanitizer (ASan) if supported" OFF)
option(USE_USAN "Build with UndefinedBehaviorSanitizer (UBSan) if supported" OFF)

if(MSVC AND USE_ASAN)
    # Remove the /RTC flags forcefully because CMake defaults them on MSVC
    set(CMAKE_C_FLAGS_DEBUG " /Zi /Ob0 /Od" CACHE INTERNAL "" FORCE)
    set(CMAKE_CXX_FLAGS_DEBUG " /Zi /Ob0 /Od" CACHE INTERNAL "" FORCE)
endif()

if(CMAKE_SYSTEM_NAME STREQUAL "Emscripten"
        AND CMAKE_BUILD_TYPE STREQUAL "Release")
    set(CMAKE_C_FLAGS_RELEASE " -O3" CACHE INTERNAL "" FORCE)
    set(CMAKE_CXX_FLAGS_RELEASE " -O3" CACHE INTERNAL "" FORCE)
endif()

add_subdirectory(vendor)
add_subdirectory(src)
