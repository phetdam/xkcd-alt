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

# executable name
set(PDXKA_PROGNAME xkcd-alt)

# use Boost.ProgramOptions for CLI argument parsing
option(PDXKA_USE_BOOST_PROGRAM_OPTIONS "Use Boost.ProgramOptions" ON)
# support library built as shared by default
option(BUILD_SHARED_LIBS "Build libraries as shared" ON)
# build tests. defaults to ON since you should build tests
option(BUILD_TESTS "Build project tests" ON)

include(CTest)

# default build type is Debug (single-config generators only)
if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE Debug)
endif()

# build type maintains case
set(PDXKA_BUILD_TYPE ${CMAKE_BUILD_TYPE})
message(STATUS "Config: ${PDXKA_BUILD_TYPE}")
# indicate pdxka library build type
if(BUILD_SHARED_LIBS)
    message(STATUS "Library build: Shared")
else()
    message(STATUS "Library build: Static")
endif()

# output artifacts into top-level directory. multi-config generators like
# Visual Studio will have an extra per-config subdirectory for the build type
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR})
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR})
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR})
# separate variable for output directory
set(PDXKA_BINARY_DIR ${CMAKE_BINARY_DIR})

# set some system information variables used for the version info
set(PDXKA_SYSTEM_NAME ${CMAKE_SYSTEM_NAME})
set(PDXKA_SYSTEM_VERSION ${CMAKE_SYSTEM_VERSION})
set(PDXKA_SYSTEM_ARCH ${CMAKE_SYSTEM_PROCESSOR})

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# use Boost.ProgramOptions for command-line option parsing. if not defined,
# then hand-rolled implementation (WIP) is to be used. the goal is to get off
# Boost.ProgramOptions so only Boost headers are needed.
if(PDXKA_USE_BOOST_PROGRAM_OPTIONS)
    message(STATUS "CLI options: Boost program_options")
    # TODO: remove macro definition here and instead define to 1 or 0 in
    # features.h generated from features.h.in
    add_compile_definitions(PDXKA_USE_BOOST_PROGRAM_OPTIONS)
else()
    message(STATUS "CLI options: Manual")
endif()

# compiler-specific options
if(MSVC)
    # /O2 added by CMake for Release, /O0 is default
    add_compile_options(/Wall)
# options are also accepted by clang
else()
    # -O0 is default
    add_compile_options(-Wall)
endif()

# cURL and Boost minimum versions
set(PDXKA_CURL_VERSION 7.68)
set(PDXKA_BOOST_VERSION 1.71)
# need to be able to make HTTPS request. note: CMake errors on Windows, as the
# generated curl-config Bash script of course cannot be run by CMD
find_package(CURL ${PDXKA_CURL_VERSION} REQUIRED COMPONENTS HTTPS)
# compiled Boost components to search for
set(PDXKA_BOOST_COMPONENTS "")
# boost_program_options is compiled
if(PDXKA_USE_BOOST_PROGRAM_OPTIONS)
    set(PDXKA_BOOST_COMPONENTS ${PDXKA_BOOST_COMPONENTS} program_options)
endif()
# boost_filesystem is required since boost::process::search_path uses Boost
# filesystem path object. unit_test_framework is also compiled
if(BUILD_TESTS)
    set(
        PDXKA_BOOST_COMPONENTS
        ${PDXKA_BOOST_COMPONENTS} filesystem unit_test_framework
    )
endif()
# sort Boost components just for vanity purposes + print as space-separated
list(SORT PDXKA_BOOST_COMPONENTS)
string(REPLACE ";" " " PDXKA_BOOST_COMPONENTS_STRING "${PDXKA_BOOST_COMPONENTS}")
message(STATUS "Boost components: ${PDXKA_BOOST_COMPONENTS_STRING}")
# find Boost headers and any components
if(PDXKA_BOOST_COMPONENTS)
    find_package(
        Boost ${PDXKA_BOOST_VERSION} REQUIRED
        COMPONENTS ${PDXKA_BOOST_COMPONENTS}
    )
else()
    find_package(Boost ${PDXKA_BOOST_VERSION} REQUIRED)
endif()

# add include dir. use SYSTEM to suppress warnings from third-party libraries
set(PDXKA_INCLUDE ${CMAKE_CURRENT_SOURCE_DIR}/include)
include_directories(
    ${PDXKA_INCLUDE}
    SYSTEM ${Boost_INCLUDE_DIRS}
    SYSTEM ${CURL_INCLUDE_DIRS}
)

# generate features.h from features.h.in with any configured features. we
# ensure that the generated header has the same LF line endings as source
set(PDXKA_FEATURES_HEADER ${PDXKA_INCLUDE}/pdxka/features.h)
configure_file(
    ${PDXKA_FEATURES_HEADER}.in ${PDXKA_FEATURES_HEADER}
    @ONLY NEWLINE_STYLE LF
)
# note: cmake_path available in CMake 3.20
cmake_path(
    RELATIVE_PATH PDXKA_FEATURES_HEADER
    BASE_DIRECTORY ${PDXKA_INCLUDE}
    OUTPUT_VARIABLE PDXKA_FEATURES_HEADER_RELATIVE
)
message(STATUS "Generated ${PDXKA_FEATURES_HEADER_RELATIVE}")

# generate version.h from version.h.in with project version/build info
set(PDXKA_VERSION_HEADER ${PDXKA_INCLUDE}/pdxka/version.h)
configure_file(
    ${PDXKA_VERSION_HEADER}.in ${PDXKA_VERSION_HEADER}
    @ONLY NEWLINE_STYLE LF
)
cmake_path(
    RELATIVE_PATH PDXKA_VERSION_HEADER
    BASE_DIRECTORY ${PDXKA_INCLUDE}
    OUTPUT_VARIABLE PDXKA_VERSION_HEADER_RELATIVE
)
message(STATUS "Generated ${PDXKA_VERSION_HEADER_RELATIVE}")

add_subdirectory(src)

if(BUILD_TESTS)
    # generate path.hh header for test paths
    set(PDXKA_TEST_PATHS_HEADER ${PDXKA_INCLUDE}/pdxka/testing/path.hh)
    configure_file(
        ${PDXKA_TEST_PATHS_HEADER}.in ${PDXKA_TEST_PATHS_HEADER}
        @ONLY NEWLINE_STYLE LF
    )
    cmake_path(
        RELATIVE_PATH PDXKA_TEST_PATHS_HEADER
        BASE_DIRECTORY ${PDXKA_INCLUDE}
        OUTPUT_VARIABLE PDXKA_TEST_PATHS_HEADER_RELATIVE
    )
    message(STATUS "Generated ${PDXKA_TEST_PATHS_HEADER_RELATIVE}")
    message(STATUS "Build tests: Enabled")
    add_subdirectory(test)
else()
    message(STATUS "Build tests: Disabled")
endif()
