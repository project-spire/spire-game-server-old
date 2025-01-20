cmake_minimum_required(VERSION 3.28)

project(
    spire_game_server
    DESCRIPTION "A game server of Project Spire"
    VERSION 25.1.18
    LANGUAGES CXX
)

option(SPIRE_BUILD_TESTS "Enable builds of tests" ON)


# Source
# ----------------------------------------------------------------
add_library(base_lib STATIC)
add_library(spire::base_lib ALIAS base_lib)

target_compile_features(base_lib PUBLIC cxx_std_23)
target_compile_options(base_lib PUBLIC -Wall -Wextra -Wpedantic)
target_include_directories(base_lib PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/src ${CMAKE_CURRENT_BINARY_DIR}/gen)
add_subdirectory(src/spire/component)
add_subdirectory(src/spire/core)
add_subdirectory(src/spire/net)
add_subdirectory(src/spire/system)

add_executable(game_server
    src/spire/main.cpp
)
target_link_libraries(game_server PRIVATE spire::base_lib)


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

FetchContent_Declare(
    EnTT
    GIT_REPOSITORY https://github.com/skypjack/entt.git
    GIT_TAG v3.14.0
)
FetchContent_MakeAvailable(EnTT)

FetchContent_Declare(
    glm
    GIT_REPOSITORY https://github.com/g-truc/glm.git
    GIT_TAG 1.0.1
)
FetchContent_MakeAvailable(glm)

# Linking
target_link_libraries(base_lib
    PUBLIC
    Boost::system
    EnTT::EnTT
    protobuf::libprotobuf
    tf::default_settings
    glm
)


# Protobuf compilations
# ----------------------------------------------------------------
set(SPIRE_LIB_MESSAGE_GEN_DIR ${CMAKE_CURRENT_BINARY_DIR}/gen/spire/msg)
set(SPIRE_LIB_MESSAGE_INCLUDE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/protocol/msg)
file(MAKE_DIRECTORY ${SPIRE_LIB_MESSAGE_GEN_DIR})
file(GLOB_RECURSE messages protocol/msg/*.proto)
message("Protobuf messages: ${messages}")

function(compile_protobuf_message in_file)
    get_filename_component(in_file_dir ${in_file} DIRECTORY)
    string(REPLACE "${SPIRE_LIB_MESSAGE_INCLUDE_DIR}" "" relative_in_file_dir "${in_file_dir}")

    get_filename_component(name ${in_file} NAME_WE)

    set(out_src "${SPIRE_LIB_MESSAGE_GEN_DIR}/${relative_in_file_dir}/${name}.pb.cc")
    set(out_hdr "${SPIRE_LIB_MESSAGE_GEN_DIR}/${relative_in_file_dir}/${name}.pb.h")

    add_custom_command(
        OUTPUT "${out_src}" "${out_hdr}"
        COMMAND protoc
        ARGS
            --cpp_out=${SPIRE_LIB_MESSAGE_GEN_DIR}
            -I${SPIRE_LIB_MESSAGE_INCLUDE_DIR}
            ${in_file}
        DEPENDS ${in_file} protoc
        COMMENT "[protoc] Compiling message: ${out_src} ${out_hdr}"
    )

    target_sources(base_lib PUBLIC ${out_src} ${out_hdr})
endfunction()

foreach(message ${messages})
    compile_protobuf_message(${message})
endforeach()

target_include_directories(base_lib PUBLIC ${SPIRE_LIB_MESSAGE_GEN_DIR})