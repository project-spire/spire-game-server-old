add_executable(ping main.cpp)
target_compile_features(ping PRIVATE cxx_std_23)
target_compile_options(ping PRIVATE -Wall -Wextra -Wpedantic)
target_link_libraries(ping PRIVATE spire::core)

set_target_properties(ping PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}
)