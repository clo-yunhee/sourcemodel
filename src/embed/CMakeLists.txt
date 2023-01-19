add_library(embed INTERFACE)
target_include_directories(embed INTERFACE ${CMAKE_CURRENT_LIST_DIR}/include)

if(DEFINED EMBED_ARRAY) # It will be defined when cross-compiling
    target_compile_definitions(embed INTERFACE "EMBED_ARRAY=${EMBED_ARRAY}")
else()
    if(MSVC)
        target_compile_definitions(embed INTERFACE "EMBED_ARRAY=0")
    else()
        target_compile_definitions(embed INTERFACE "EMBED_ARRAY=1")
    endif()
endif()

#embed_cli: if cross compiling needs to be built as a host executable
if(CMAKE_CROSSCOMPILING)
    include(ExternalProject)
    ExternalProject_Add(
        embed_cli
        SOURCE_DIR ${CMAKE_CURRENT_SOURCE_DIR}
        BINARY_DIR ${CMAKE_CURRENT_BINARY_DIR}
        UPDATE_DISCONNECTED TRUE
        DOWNLOAD_COMMAND ""
        CMAKE_ARGS -DEMBED_ARRAY=0
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

function(_add_embedded_file _target _actual_input_file _actual_output_file _variable)

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
    target_link_libraries(${_target} PUBLIC embed) # PUBLIC dep to share compile defs
    add_dependencies(${_target} embed_cli)
endfunction()

function(add_embedded_file _target _input_file _output_file _variable)
    set(_actual_input_file "${CMAKE_CURRENT_SOURCE_DIR}/${_input_file}")
    set(_actual_output_file "${CMAKE_CURRENT_BINARY_DIR}/include/embeds/${_output_file}")
    _add_embedded_file(${_target} ${_actual_input_file} ${_actual_output_file} ${_variable})
endfunction()

#add_executable(embed_test test.cpp)
#add_embedded_file(embed_test test.dat test_dat.h gTest)

# Embed FONTS specifically, using fontTools subsetter as a pre-processing pass.
function(add_embedded_font _target _input_file _output_file _variable _unicodes)
    set(_actual_input_file "${CMAKE_CURRENT_SOURCE_DIR}/${_input_file}")
    set(_actual_output_file "${CMAKE_CURRENT_BINARY_DIR}/include/embeds/${_output_file}")
    
    # Add -subset suffix
    # add_embedded_file takes a filename relative to CURRENT_SOURCE_DIR
    get_filename_component(_input_file_name ${_input_file} NAME_WE)
    set(_subset_file "${_input_file_name}-subset.woff")

    set(_subset_output_file "${CMAKE_CURRENT_BINARY_DIR}/fonts/${_subset_file}")
    add_custom_command(
        OUTPUT "${_subset_output_file}"
        COMMAND "pyftsubset"
                        "${_actual_input_file}"
                        "--unicodes=${_unicodes}"
                        "--no-ignore-missing-unicodes" 
                        "--output-file=${_subset_output_file}"
                        "--flavor=woff"
                        "--with-zopfli"
        DEPENDS "${_actual_input_file}"
        VERBATIM
    )

    _add_embedded_file(${_target} ${_subset_output_file} ${_actual_output_file} ${_variable})
endfunction()