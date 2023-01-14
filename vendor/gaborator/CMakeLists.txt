FetchContent_Declare(
    gaborator
    URL      https://download.gaborator.com/gaborator/gaborator-1.7.tar.gz
    URL_HASH SHA256=7027D71D8DB7EFD1949F3A8DE134D2D43FD57E268AA808A9E177EA86F2D20F3C
)

if(NOT gaborator_POPULATED)
    FetchContent_Populate(gaborator)

    add_library(gaborator INTERFACE)
    target_include_directories(gaborator INTERFACE ${gaborator_SOURCE_DIR})
endif()