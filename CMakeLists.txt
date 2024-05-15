cmake_minimum_required(VERSION 3.21)

##
# How to build:
#
# *nix:
#   cmake -S . -B build && cmake --build build -j
#
# Windows:
#   cmake -S . -B build_windows -G Ninja && cmake --build build_windows -j
#
# Note on Windows you should use the correct Developer Prompt, i.e. the default
# Developer Prompt for x86 output, x64 Native Tools prompt for x64 output
#

# set major, minor, patch versions + version string
set(PDXKA_MAJOR_VERSION 0)
set(PDXKA_MINOR_VERSION 1)
set(PDXKA_PATCH_VERSION 0)
set(
    PDXKA_VERSION
    ${PDXKA_MAJOR_VERSION}.${PDXKA_MINOR_VERSION}.${PDXKA_PATCH_VERSION}
)

project(
    xkcd-alt
    VERSION ${PDXKA_VERSION}
    DESCRIPTION "CLI tool to get the latest XKCD alt text"
    HOMEPAGE_URL "https://github.com/phetdam/xkcd-alt"
    LANGUAGES CXX
)

# use Boost.ProgramOptions for CLI argument parsing
option(PDXKA_USE_BOOST_PROGRAM_OPTIONS "Use Boost.ProgramOptions" ON)

# default build type is Debug (single-config generators only)
if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE Debug)
endif()

# build type maintains case
set(PDXKA_BUILD_TYPE ${CMAKE_BUILD_TYPE})
message(STATUS "Config: ${PDXKA_BUILD_TYPE}")

# output artifacts into top-level directory. multi-config generators like
# Visual Studio will have an extra per-config subdirectory for the build type
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR})
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR})
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR})

# set some system information variables used for the version info
set(PDXKA_SYSTEM_NAME ${CMAKE_SYSTEM_NAME})
set(PDXKA_SYSTEM_VERSION ${CMAKE_SYSTEM_VERSION})
set(PDXKA_SYSTEM_ARCH ${CMAKE_SYSTEM_PROCESSOR})

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# add version definitions as macro definitions
add_compile_definitions(
    PDXKA_MAJOR_VERSION=${PDXKA_MAJOR_VERSION}
    PDXKA_MINOR_VERSION=${PDXKA_MINOR_VERSION}
    PDXKA_PATCH_VERSION=${PDXKA_PATCH_VERSION}
    # need double quotes for CMake to pass as a string
    PDXKA_BUILD_TYPE="${PDXKA_BUILD_TYPE}"
    PDXKA_SYSTEM_NAME="${PDXKA_SYSTEM_NAME}"
    PDXKA_SYSTEM_VERSION="${PDXKA_SYSTEM_VERSION}"
    PDXKA_SYSTEM_ARCH="${PDXKA_SYSTEM_ARCH}"
)
# use Boost.ProgramOptions for command-line option parsing. if not defined,
# then hand-rolled implementation (WIP) is to be used. the goal is to get off
# Boost.ProgramOptions so only Boost headers are needed.
if(PDXKA_USE_BOOST_PROGRAM_OPTIONS)
    message(STATUS "CLI options: Boost program_options")
    add_compile_definitions(PDXKA_USE_BOOST_PROGRAM_OPTIONS)
else()
    message(STATUS "CLI options: Manual")
endif()

# compiler-specific options
if(MSVC)
    # /O2 added by CMake for Release, /O0 is default
    add_compile_options(/Wall $<$<CONFIG:Debug>:/DEBUG>)
# options are also accepted by clang
else()
    # -O0 is default
    add_compile_options(-Wall $<IF:$<CONFIG:Release>,-O3,-ggdb>)
endif()

# cURL and Boost minimum versions
set(PDXKA_CURL_VERSION 7.68)
set(PDXKA_BOOST_VERSION 1.71)
# need to be able to make HTTPS request. note: CMake errors on Windows, as the
# generated curl-config Bash script of course cannot be run by CMD
find_package(CURL ${PDXKA_CURL_VERSION} REQUIRED COMPONENTS HTTPS)
# program_options is not header-only, but require only when necessary
if(PDXKA_USE_BOOST_PROGRAM_OPTIONS)
    find_package(Boost ${PDXKA_BOOST_VERSION} REQUIRED COMPONENTS program_options)
else()
    find_package(Boost ${PDXKA_BOOST_VERSION} REQUIRED)
endif()

# include dir and tool name
set(PDXKA_INCLUDE ${CMAKE_CURRENT_SOURCE_DIR}/include)
set(PDXKA_PROGNAME xkcd-alt)

# use SYSTEM to suppress warnings from third-party libraries
include_directories(
    ${PDXKA_INCLUDE}
    SYSTEM ${Boost_INCLUDE_DIRS}
    SYSTEM ${CURL_INCLUDE_DIRS}
)

# TODO: generate version.h and program name instead of using macros + argv[0]

add_subdirectory(src)
