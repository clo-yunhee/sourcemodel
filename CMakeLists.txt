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

option(USE_ASAN "Build with AddressSanitizer (ASan) if supported" OFF)
option(USE_USAN "Build with UndefinedBehaviorSanitizer (UBSan) if supported" OFF)

if(CMAKE_SYSTEM_NAME STREQUAL "Windows")
    add_definitions(-D_WIN32_WINNT=0x0601)
endif()

add_subdirectory(vendor)
add_subdirectory(src)
