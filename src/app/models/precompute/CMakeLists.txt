set(_target precomputeExec)

add_executable(${_target} EXCLUDE_FROM_ALL
    precompute.cpp
)
target_link_libraries(${_target} PRIVATE
    Boost::math
    Boost::multiprecision
)
set_property(TARGET ${_target} PROPERTY CXX_STANDARD 20)
set_property(TARGET ${_target} PROPERTY CXX_STANDARD_REQUIRED TRUE)