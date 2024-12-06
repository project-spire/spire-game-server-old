cmake_minimum_required(VERSION 3.28)

project(
    spire-main
    DESCRIPTION "A main server for Project Spire"
    LANGUAGES CXX
)

option(SPIRE_BUILD_TESTS "Enable the build of tests" ON)

# Dependencies
include(FetchContent)

find_package(Boost 1.86.0 REQUIRED COMPONENTS system)

find_package(OpenSSL REQUIRED)

FetchContent_Declare(
    protobuf
    GIT_REPOSITORY https://github.com/protocolbuffers/protobuf.git
    GIT_TAG v28.3
)
FetchContent_MakeAvailable(protobuf)
option(protobuf_INSTALL "Install protobuf binaries and files" OFF)
option(protobuf_BUILD_TESTS "Build tests" OFF)

# Source
add_library(spire-lib STATIC
    spire/core/numbers.hpp
    spire/core/random.hpp
    spire/core/units.hpp
)

target_compile_features(spire-lib PUBLIC cxx_std_23)
target_compile_options(spire-lib PRIVATE -Wall -Wextra -Wpedantic)

target_include_directories(spire-lib PUBLIC ${CMAKE_CURRENT_SOURCE_DIR} ${CMAKE_CURRENT_BINARY_DIR}/spire)

target_link_libraries(spire-lib
    PRIVATE
    Boost::system
    protobuf::libprotobuf
)

add_executable(spire-server
    spire/main.cpp
)
target_link_libraries(spire-server PRIVATE spire-lib)

# Protobuf compilers
set(message_outdir ${CMAKE_CURRENT_BINARY_DIR}/spire/spire/message)
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

