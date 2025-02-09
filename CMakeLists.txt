cmake_minimum_required(VERSION 3.28)

project(spire-game-server LANGUAGES CXX)

option(SPIRE_BUILD_TESTS "Enable builds of tests" ON)


# External Dependencies
# ----------------------------------------------------------------
find_package(Boost REQUIRED COMPONENTS system)
find_package(EnTT CONFIG REQUIRED)
find_package(glm CONFIG REQUIRED)
find_package(jwt-cpp CONFIG REQUIRED)
find_package(mongocxx CONFIG REQUIRED)
find_package(OpenSSL CONFIG REQUIRED)
find_package(protobuf CONFIG REQUIRED)
find_package(spdlog CONFIG REQUIRED)
find_package(Taskflow CONFIG REQUIRED)
find_package(yaml-cpp CONFIG REQUIRED)


# Source
# ----------------------------------------------------------------

# Core
add_library(core STATIC)
add_library(spire::core ALIAS core)
target_compile_features(core PUBLIC cxx_std_23)
target_compile_options(core PUBLIC -Wall -Wextra -Wpedantic)
target_compile_definitions(core
    PUBLIC SPIRE_SETTINGS_FILE="${CMAKE_CURRENT_SOURCE_DIR}/settings.yaml"
)

target_include_directories(core PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/src ${CMAKE_CURRENT_SOURCE_DIR}/gen)
add_subdirectory(src/spire/container)
add_subdirectory(src/spire/core)
add_subdirectory(src/spire/net)

target_link_libraries(core
    PUBLIC
    Boost::system
    OpenSSL::Crypto
    OpenSSL::SSL
    protobuf::libprotobuf
    spdlog::spdlog
    yaml-cpp::yaml-cpp
)

# Server
add_executable(server src/spire/main.cpp)
target_compile_features(server PRIVATE cxx_std_23)
target_compile_options(server PRIVATE -Wall -Wextra -Wpedantic)

add_subdirectory(src/spire/component)
add_subdirectory(src/spire/handler)
add_subdirectory(src/spire/server)
add_subdirectory(src/spire/system)
add_subdirectory(src/spire/room)

target_link_libraries(server
    PRIVATE
    spire::core

    EnTT::EnTT
    glm::glm
    jwt-cpp::jwt-cpp
    mongo::mongocxx_static
    Taskflow::Taskflow
)


# Protobuf compilations
# ----------------------------------------------------------------
find_program(PROTOC_EXECUTABLE NAMES protoc REQUIRED)

set(SPIRE_MESSAGE_GEN_DIR ${CMAKE_CURRENT_SOURCE_DIR}/gen/spire/msg)
set(SPIRE_MESSAGE_INCLUDE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/protocol/msg)
file(MAKE_DIRECTORY ${SPIRE_MESSAGE_GEN_DIR})
file(GLOB_RECURSE messages protocol/msg/*.proto)
message("Protobuf messages: ${messages}")

function(compile_protobuf_message in_file)
    get_filename_component(in_file_dir ${in_file} DIRECTORY)
    string(REPLACE "${SPIRE_MESSAGE_INCLUDE_DIR}" "" relative_in_file_dir "${in_file_dir}")

    get_filename_component(name ${in_file} NAME_WE)

    set(out_src "${SPIRE_MESSAGE_GEN_DIR}/${relative_in_file_dir}/${name}.pb.cc")
    set(out_hdr "${SPIRE_MESSAGE_GEN_DIR}/${relative_in_file_dir}/${name}.pb.h")

    add_custom_command(
        OUTPUT "${out_src}" "${out_hdr}"
        COMMAND ${PROTOC_EXECUTABLE}
        ARGS
            --cpp_out=${SPIRE_MESSAGE_GEN_DIR}
            -I${SPIRE_MESSAGE_INCLUDE_DIR}
            ${in_file}
        COMMENT "[protoc] Compiling message: ${out_src} ${out_hdr}"
    )

    target_sources(core PUBLIC ${out_src} ${out_hdr})
endfunction()

foreach(message ${messages})
    compile_protobuf_message(${message})
endforeach()
target_include_directories(core PUBLIC ${SPIRE_MESSAGE_GEN_DIR})


# Tool
# ----------------------------------------------------------------
add_subdirectory(tool/ping)