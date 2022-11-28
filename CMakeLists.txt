cmake_minimum_required(VERSION 3.21)

##
# How to build:
#
# *nix:
#   cmake -S . -B build [-DUSE_CURL=1] [-DUSE_BEAST=1] && cmake --build build
#
# Windows:
#   cmake -S . -B build_windows -G Ninja [-DUSE_CURL=1] [-DUSE_BEAST=1] && ^
#   cmake --build build_windows
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

option(USE_BEAST "Use Boost.Beast to make HTTP request" OFF)
option(USE_CURL "Use libcurl to make HTTP request" OFF)

if(NOT DEFINED CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE Debug)
endif()

# USE_BEAST and USE_CURL are mutually exclusive, but USE_BEAST should be set to
# ON by default if neither are specified when building
if(NOT USE_BEAST AND NOT USE_CURL)
    set(USE_BEAST ON CACHE BOOL "" FORCE)
elseif(USE_BEAST AND USE_CURL)
    message(FATAL_ERROR "USE_BEAST and USE_CURL are mutually exclusive")
endif()

# update preprocessor defines
if(USE_BEAST)
    add_compile_definitions(PDXKA_USE_BEAST)
elseif(USE_CURL)
    add_compile_definitions(PDXKA_USE_CURL)
# should never hit this
else()
    message(FATAL_ERROR "neither USE_BEAST nor USE_CURL defined!")
endif()

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

if(USE_CURL)
    find_package(CURL 7.68 REQUIRED)
    set(PDXKA_HTTP_LIBRARY CURL::libcurl)
else()
    # target for Boost headers, won't actually link to anything
    set(PDXKA_HTTP_LIBRARY Boost::boost)
endif()

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

# Boost.Beast is header only
find_package(Boost 1.71 REQUIRED COMPONENTS program_options)

# include dir and tool name
set(PDXKA_INCLUDE ${CMAKE_CURRENT_SOURCE_DIR}/include)
set(PDXKA_PROGNAME xkcd-alt)

# use SYSTEM to suppress warnings from third-party libraries
include_directories(
    ${PDXKA_INCLUDE}
    SYSTEM ${Boost_INCLUDE_DIRS}
)

add_subdirectory(src)
# add_subdirectory(test)
