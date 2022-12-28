add_library(embed INTERFACE)
target_include_directories(embed INTERFACE ${CMAKE_CURRENT_LIST_DIR}/include)

#embed_cli: if cross compiling needs to be built as a host executable
if(CMAKE_CROSSCOMPILING)
    include(ExternalProject)
    ExternalProject_Add(
        embed_cli
        SOURCE_DIR ${CMAKE_CURRENT_SOURCE_DIR}
        BINARY_DIR ${CMAKE_CURRENT_BINARY_DIR}
        UPDATE_DISCONNECTED TRUE
        DOWNLOAD_COMMAND ""
        BUILD_COMMAND ${CMAKE_COMMAND} --build ${CMAKE_CURRENT_BINARY_DIR} --target embed_cli
        INSTALL_COMMAND ""
    )

    if(CMAKE_HOST_SYSTEM_NAME STREQUAL "Windows")
        set(CMAKE_HOST_EXECUTABLE_SUFFIX .exe)
    endif()
    set(embed_cli_exe ${CMAKE_CURRENT_BINARY_DIR}/embed_cli${CMAKE_HOST_EXECUTABLE_SUFFIX} PARENT_SCOPE)
else()
    add_executable(embed_cli main.cpp)
    target_include_directories(embed_cli PUBLIC embed)

    set_property(TARGET embed_cli PROPERTY CXX_STANDARD 20)
    set_property(TARGET embed_cli PROPERTY CXX_STANDARD_REQUIRED TRUE)

    if(CMAKE_SYSTEM_NAME STREQUAL "Windows")
        target_compile_definitions(embed_cli PRIVATE _CRT_SECURE_NO_WARNINGS)
    endif()

    set(embed_cli_exe "$<TARGET_FILE:embed_cli>" PARENT_SCOPE)
endif()

function(add_embedded_file _target _input_file _output_file _variable)
    set(_sub_target "${_target}_${_variable}")
    set(_actual_input_file "${CMAKE_CURRENT_SOURCE_DIR}/${_input_file}")
    set(_actual_output_file "${CMAKE_CURRENT_BINARY_DIR}/include/embeds/${_output_file}")

    add_custom_command(
        OUTPUT "${_actual_output_file}"
        COMMAND "${embed_cli_exe}"
                        "${_actual_input_file}"
                        "${_actual_output_file}"
                        "${_variable}"
        DEPENDS "${_actual_input_file}"
        VERBATIM
    )
    target_sources(${_target} PRIVATE "${_actual_output_file}")
    set_source_files_properties(${_actual_output_file} TARGET_DIRECTORY ${_target} PROPERTIES GENERATED TRUE)
    target_include_directories(${_target} PRIVATE ${CMAKE_CURRENT_BINARY_DIR}/include)
    target_link_libraries(${_target} PRIVATE embed)
    add_dependencies(${_target} embed_cli)
endfunction()

#add_executable(embed_test test.cpp)
#add_embedded_file(embed_test test.dat test_dat.h gTest)
