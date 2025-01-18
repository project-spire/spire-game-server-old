cmake_minimum_required(VERSION 3.28)

project(
    spire-game-server
    DESCRIPTION "A game server of Project Spire"
    VERSION 25.1.18
    LANGUAGES CXX
)

option(SPIRE_BUILD_TESTS "Enable builds of tests" ON)


# Source
# ----------------------------------------------------------------
add_library(spire-lib STATIC)
target_compile_features(spire-lib PUBLIC cxx_std_23)
target_compile_options(spire-lib PRIVATE -Wall -Wextra -Wpedantic)
target_include_directories(spire-lib PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/src ${CMAKE_CURRENT_BINARY_DIR}/src)
add_subdirectory(src/spire/core)
add_subdirectory(src/spire/net)

add_executable(spire-game-server
    src/spire/main.cpp
)
target_compile_options(spire-game-server PRIVATE -Wall -Wextra -Wpedantic)
target_link_libraries(spire-game-server PRIVATE spire-lib)


# External Dependencies
# ----------------------------------------------------------------
include(FetchContent)

find_package(Boost 1.87.0 REQUIRED COMPONENTS system)

find_package(OpenSSL REQUIRED)

FetchContent_Declare(
    protobuf
    GIT_REPOSITORY https://github.com/protocolbuffers/protobuf.git
    GIT_TAG v29.3
)
FetchContent_MakeAvailable(protobuf)
option(protobuf_INSTALL "Install protobuf binaries and files" OFF)
option(protobuf_BUILD_TESTS "Build tests" OFF)

FetchContent_Declare(
    tf
    GIT_REPOSITORY https://github.com/taskflow/taskflow.git
    GIT_TAG v3.9.0
)
FetchContent_MakeAvailable(tf)
option(TF_BUILD_TESTS "Enables builds of tests" OFF)
option(TF_BUILD_EXAMPLES "Enables builds of examples" OFF)

# Linking
target_link_libraries(spire-lib
    PRIVATE
    Boost::system
    protobuf::libprotobuf
    tf::default_settings
)


# Protobuf compilations
# ----------------------------------------------------------------
set(message_outdir ${CMAKE_CURRENT_BINARY_DIR}/src/spire/message)
file(MAKE_DIRECTORY ${message_outdir})
file(GLOB_RECURSE messages messages/*.proto)

function(protobuf_compiler infile include outdir)
    get_filename_component(name ${infile} NAME_WE)

    set(outfile
        "${outdir}/${name}pb.cc"
        "${outdir}/${name}pb.h"
    )

    add_custom_command(
        OUTPUT ${outfile}
        COMMAND protoc
        ARGS
        --cpp_out=${outdir}
        -I${include}
        ${infile}
        DEPENDS ${infile} protoc
        COMMENT "[protoc] Compiling message: ${out_file}"
        VERBATIM
    )

    add_custom_target(${name}-proto ALL DEPENDS ${outfile})
    add_dependencies(tower-lib ${name}-proto)
endfunction()

foreach(message ${messages})
    protobuf_compiler(${message} ${CMAKE_CURRENT_SOURCE_DIR}/messages ${message_outdir})
endforeach()

