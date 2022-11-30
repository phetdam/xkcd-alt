cmake_minimum_required(VERSION 3.21)

##
# How to build:
#
# *nix:
#   cmake -S . -B build && cmake --build build
#
# Windows:
#   cmake -S . -B build_windows -G Ninja && cmake --build build_windows
#
# Note on Windows you should use the correct Developer Prompt, i.e. the default
# Developer Prompt for x86 output, x64 Native Tools prompt for x64 output
#

project(
    xkcd-alt
    VERSION 0.0.1
    DESCRIPTION "CLI tool to get the daily XKCD alt text one-liner"
    HOMEPAGE_URL "https://github.com/phetdam/xkcd-alt"
    LANGUAGES CXX
)

# default build type is Debug
if(NOT DEFINED CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE Debug)
endif()

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

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
# add_subdirectory(test)
