cmake_minimum_required(VERSION 3.21)

##
# How to build:
#
# *nix:
#   cmake -S . -B build && cmake --build build -j
#
# Windows:
#   cmake -S . -B build_windows_x64 -A x64 -DBoost_USE_STATIC_LIBS=OFF ^
#       -DBUILD_STATIC_LIBS=OFF
#   cmake --build build_windows -config Debug -j
#
# Note on Windows you should use the Developer Command Prompt.
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

# support library built as shared by default
# TODO: this should be set to OFF for Windows until we have DLL export ready
option(BUILD_SHARED_LIBS "Build libraries as shared" ON)
# build tests. defaults to ON since you should build tests
option(BUILD_TESTS "Build project tests" ON)
# enable AddressSanitizer
option(ENABLE_ASAN "Enable AddressSanitizer instrumentation" OFF)
# use Boost.ProgramOptions for CLI argument parsing
option(PDXKA_USE_BOOST_PO_CLI "Use Boost.ProgramOptions" ON)

# determine if generator is multi-config
get_property(PDXKA_IS_MULTI_CONFIG GLOBAL PROPERTY GENERATOR_IS_MULTI_CONFIG)
if(PDXKA_IS_MULTI_CONFIG)
    message(STATUS "Generator config: Multi")
else()
    message(STATUS "Generator config: Single")
endif()

include(CTest)

# default build type is Debug (single-config generators only)
# FIXME: for multi-config generators this will be useless but is still used by
# configure_file to generate version.h unfortunately
if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE Debug)
endif()

# build type maintains case
set(PDXKA_BUILD_TYPE ${CMAKE_BUILD_TYPE})
if(NOT PDXKA_IS_MULTI_CONFIG)
    message(STATUS "Build config: ${PDXKA_BUILD_TYPE}")
endif()
# indicate pdxka library build type
if(BUILD_SHARED_LIBS)
    message(STATUS "Library build: Shared")
else()
    message(STATUS "Library build: Static")
endif()
# indicate if tests are being built
if(BUILD_TESTS)
    message(STATUS "Build tests: Yes")
else()
    message(STATUS "Build tests: No")
endif()
# AddressSanitizer on or off
if(ENABLE_ASAN)
    message(STATUS "Enable ASan: Yes")
else()
    message(STATUS "Enable ASan: No")
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

# note: PDXKA_USE_BOOST_PROGRAM_OPTIONS is used in features.h by configure_file
# so we have to set a normal CMake variable from PDXKA_USE_BOOST_PO_CLI
set(PDXKA_USE_BOOST_PROGRAM_OPTIONS ${PDXKA_USE_BOOST_PO_CLI})
# use Boost.ProgramOptions for command-line option parsing. if not defined,
# then hand-rolled implementation (WIP) is to be used. the goal is to get off
# Boost.ProgramOptions so only Boost headers are needed.
if(PDXKA_USE_BOOST_PROGRAM_OPTIONS)
    message(STATUS "CLI options: Boost")
else()
    message(STATUS "CLI options: Manual")
endif()

# compiler-specific options
if(MSVC)
    # /O2 added by CMake for Release, /O0 is default
    add_compile_options(
        /Wall
        # C4355: this used in base initializer list (pplwin.h)
        /wd4355
        # C4514: unreferenced inline function removed
        /wd4514
        # C4623: default ctor implicitly defined as deleted
        /wd4623
        # C4626: assignment operator implicitly defined as deleted
        /wd4626
        # C4710: function not inlined
        /wd4710
        # C4711: function selected for automatic inline expansion
        /wd4711
        # C4820: padding added after data member
        /wd4820
        # C5027: move assignment operator implicitly defined as deleted
        /wd5027
        # C5045: compiler will insert Spectre mitigation with /QSpectre
        /wd5045
    )
# options are also accepted by clang
else()
    # -O0 is default
    add_compile_options(-Wall)
    if(ENABLE_ASAN)
        add_compile_options(-fsanitize=address)
        add_link_options(-fsanitize=address)
    endif()
endif()

# add cmake directory for modules + include pdxka_find_curl macro
list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/cmake")
include(pdxka_find_curl)

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
set(PDXKA_BOOST_VERSION 1.71)
# if compiled components are requested
if(PDXKA_BOOST_COMPONENTS)
    find_package(
        Boost ${PDXKA_BOOST_VERSION} REQUIRED
        COMPONENTS ${PDXKA_BOOST_COMPONENTS}
    )
else()
    find_package(Boost ${PDXKA_BOOST_VERSION} REQUIRED)
endif()
message(STATUS "Boost version: ${Boost_VERSION_STRING}")
# need to be able to make HTTPS request. note: CMake errors on Windows, as the
# generated curl-config Bash script of course cannot be run by CMD
set(PDXKA_CURL_VERSION 7.68)
# when not on Windows, a standard find_package call works
if(NOT WIN32)
    find_package(CURL ${PDXKA_CURL_VERSION} REQUIRED COMPONENTS HTTPS)
# as of 8.11.2-DEV libcurl's CMake config doesn't work correctly, so we need
# special logic. we can't use curl-config (it's a Bash script), but we can get
# a lot of info from the curl -V version output (features + components).
else()
    pdxka_find_curl(VERSION ${PDXKA_CURL_VERSION} REQUIRED COMPONENTS HTTPS)
endif()
# note: this is set by FindCURL. if using CONFIG mode use CURL_VERSION
message(STATUS "libcurl version: ${CURL_VERSION_STRING}")

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
# FIXME: consider generating the header in the binary directory as the contents
# of version.h differ when running on WSL Ubuntu vs. on native Windows. this
# leads to needly recompilation when switching between the two.
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

# test or other data directory root
set(PDXKA_DATA_DIR ${CMAKE_CURRENT_SOURCE_DIR}/data)

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
    add_subdirectory(test)
endif()
