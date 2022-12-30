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

# default build type is Debug
if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE Debug)
endif()

# we use lowercase build type in the project
string(TOLOWER ${CMAKE_BUILD_TYPE} PDXKA_BUILD_TYPE)

# set some system information variables used for the version info
set(PDKXA_SYSTEM_NAME ${CMAKE_SYSTEM_NAME})
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
    PDXKA_SYSTEM_NAME="${PDKXA_SYSTEM_NAME}"
    PDXKA_SYSTEM_VERSION="${PDXKA_SYSTEM_VERSION}"
    PDXKA_SYSTEM_ARCH="${PDXKA_SYSTEM_ARCH}"
)

# compiler-specific options
if(MSVC)
    # CMake adds /O2 by default for release version
    if(NOT CMAKE_BUILD_TYPE STREQUAL Release)
        add_compile_options(/Od /DEBUG)
    endif()
# options are also accepted by clang
else()
    add_compile_options(-Wall)
    if(CMAKE_BUILD_TYPE STREQUAL Release)
        add_compile_options(-O3)
    else()
        add_compile_options(-O0 -ggdb)
    endif()
endif()

# need to be able to make HTTPS request
find_package(CURL 7.68 REQUIRED COMPONENTS HTTPS)
# program_options is not header-only
find_package(Boost 1.71 REQUIRED COMPONENTS program_options)

# include dir and tool name
set(PDXKA_INCLUDE ${CMAKE_CURRENT_SOURCE_DIR}/include)
set(PDXKA_PROGNAME xkcd-alt)

# use SYSTEM to suppress warnings from third-party libraries
include_directories(
    ${PDXKA_INCLUDE}
    SYSTEM ${Boost_INCLUDE_DIRS}
    SYSTEM ${CURL_INCLUDE_DIRS}
)

add_subdirectory(src)
