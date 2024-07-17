cmake_minimum_required(VERSION 3.21)
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_OSX_ARCHITECTURES "arm64;x86_64")

project(DevTools VERSION 1.0.0)

if(${GEODE_TARGET_PLATFORM} STREQUAL "iOS" OR IOS)
  message(STATUS "building for ios :3")
  unset(CMAKE_OSX_ARCHITECTURES)
  set(CMAKE_OSX_ARCHITECTURES arm64)
  set(CMAKE_OSX_DEPLOYMENT_TARGET "14.0")
  set(CMAKE_SYSTEM_NAME "iOS")
endif()

file(GLOB_RECURSE SOURCES
    src/*.cpp
)

add_library(${PROJECT_NAME} SHARED ${SOURCES} src/platform/Mac.mm)

set_source_files_properties(src/platform/Mac.mm PROPERTIES SKIP_PRECOMPILE_HEADERS ON)

if (NOT DEFINED ENV{GEODE_SDK})
    message(FATAL_ERROR "Unable to find Geode SDK! Please define GEODE_SDK environment variable to point to Geode")
else()
    message(STATUS "Found Geode: $ENV{GEODE_SDK}")
endif()

add_subdirectory($ENV{GEODE_SDK} ${CMAKE_CURRENT_BINARY_DIR}/geode)

CPMAddPackage("gh:ocornut/imgui#c191faf")

target_include_directories(${PROJECT_NAME} PRIVATE ${imgui_SOURCE_DIR})

target_sources(${PROJECT_NAME} PRIVATE
    ${imgui_SOURCE_DIR}/imgui.cpp
    ${imgui_SOURCE_DIR}/imgui_widgets.cpp
    ${imgui_SOURCE_DIR}/imgui_tables.cpp
    ${imgui_SOURCE_DIR}/imgui_draw.cpp
    ${imgui_SOURCE_DIR}/imgui_demo.cpp
    ${imgui_SOURCE_DIR}/misc/cpp/imgui_stdlib.cpp
)

# i still dont like this (alk)
target_compile_definitions(geode-sdk INTERFACE GEODE_EXPOSE_SECRET_INTERNALS_IN_HEADERS_DO_NOT_DEFINE_PLEASE)

# Set up dependencies, resources, link Geode
setup_geode_mod(${PROJECT_NAME})
