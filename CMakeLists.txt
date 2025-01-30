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
#   cmake --build build_windows --config Debug -j
#
# Note on Windows you should use the Developer Command Prompt.
#

project(
    XkcdAlt
    VERSION 0.1.0
    DESCRIPTION "CLI tool to get the latest XKCD alt text"
    HOMEPAGE_URL "https://github.com/phetdam/xkcd-alt"
    LANGUAGES CXX
)

# executable name
set(PDXKA_PROGNAME xkcd-alt)

# support library built as shared by default
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
    message(STATUS "Build config: Multi")
# for single-config, set CMAKE_BUILD_TYPE and PDXKA_BUILD_TYPE (used by
# configure_file to configure version.h)
else()
    if(NOT CMAKE_BUILD_TYPE)
        set(CMAKE_BUILD_TYPE Debug)
    endif()
    set(PDXKA_BUILD_TYPE ${CMAKE_BUILD_TYPE})
    message(STATUS "Build config: ${PDXKA_BUILD_TYPE}")
endif()

include(CTest)

# indicate pdxka library build type
if(BUILD_SHARED_LIBS)
    message(STATUS "Build libraries: Shared")
else()
    message(STATUS "Build libraries: Static")
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
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR})
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR})
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR})
# separate variable for output directory
# FIXME: deprecate this; not accurate for multi-config generators
set(PDXKA_BINARY_DIR ${CMAKE_CURRENT_BINARY_DIR})

# set version variables
set(PDXKA_MAJOR_VERSION ${PROJECT_VERSION_MAJOR})
set(PDXKA_MINOR_VERSION ${PROJECT_VERSION_MINOR})
set(PDXKA_PATCH_VERSION ${PROJECT_VERSION_PATCH})
set(PDXKA_VERSION ${PROJECT_VERSION})
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

# add cmake directory for modules + include CMake helpers
set(PDXKA_CMAKE_MODULE_DIR "${CMAKE_CURRENT_SOURCE_DIR}/cmake")
list(APPEND CMAKE_MODULE_PATH "${PDXKA_CMAKE_MODULE_DIR}")
include(pdxka_find_curl)
include(pdxka_target_helpers)
include(pdxka_test_helpers)

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
# for multi-config generators use binary dir generator expression as the
# generated files will go in the per-config subdirectories
if(PDXKA_IS_MULTI_CONFIG)
    include_directories(${CMAKE_CURRENT_BINARY_DIR}/$<CONFIG>/include)
# for single-config generator generated files are from binary dir
else()
    include_directories(${CMAKE_CURRENT_BINARY_DIR}/include)
endif()

# generate features.h from features.h.in with any configured features. we
# ensure that the generated header has the same LF line endings as source
# TODO: write features.h to build directories so Windows/Linux builds don't
# interfere with each other. will need ${CMAKE_CURRENT_BINARY_DIR}/include to
# be on the list of include directories unconditionally
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
set(PDXKA_VERSION_HEADER pdxka/version.h)
# for multi-config generators, this must be a pre-build step, so we use a
# custom target to ensure that this step can be done before pdxka building
# note: if version.h will become a public header we will probably want to also
# generate a separate config.hh that will contain the non-version information
if(PDXKA_IS_MULTI_CONFIG)
    # helper path
    set(_version_h_path "include/${PDXKA_VERSION_HEADER}")
    add_custom_target(
        pdxka_version_h
        COMMAND
            ${CMAKE_COMMAND}
                -DPDXKA_VERSION_HEADER_IN=${PDXKA_INCLUDE}/${PDXKA_VERSION_HEADER}.in
                # generator expression for correct per-config output
                -DPDXKA_VERSION_HEADER=${CMAKE_CURRENT_BINARY_DIR}/$<CONFIG>/${_version_h_path}
                # variables consumed by version.h
                -DPDXKA_MAJOR_VERSION=${PDXKA_MAJOR_VERSION}
                -DPDXKA_MINOR_VERSION=${PDXKA_MINOR_VERSION}
                -DPDXKA_PATCH_VERSION=${PDXKA_PATCH_VERSION}
                -DPDXKA_VERSION=${PDXKA_VERSION}
                -DPDXKA_BUILD_TYPE=$<CONFIG>
                -DPDXKA_SYSTEM_NAME=${PDXKA_SYSTEM_NAME}
                -DPDXKA_SYSTEM_VERSION=${PDXKA_SYSTEM_VERSION}
                -DPDXKA_SYSTEM_ARCH=${PDXKA_SYSTEM_ARCH}
                -DPDXKA_PROGNAME=${PDXKA_PROGNAME}
                -DBoost_VERSION_MACRO=${Boost_VERSION_MACRO}
                -DBoost_VERSION_STRING=${Boost_VERSION_STRING}
                -DBoost_MAJOR_VERSION=${Boost_MAJOR_VERSION}
                -DBoost_MINOR_VERSION=${Boost_MINOR_VERSION}
                -DBoost_SUBMINOR_VERSION=${Boost_SUBMINOR_VERSION}
                -DCURL_VERSION_STRING=${CURL_VERSION_STRING}
                -P "${PDXKA_CMAKE_MODULE_DIR}/pdxka_configure_version.cmake"
        BYPRODUCTS "${CMAKE_CURRENT_BINARY_DIR}/$<CONFIG>/${_version_h_path}"
        COMMENT "Generating $<CONFIG> ${PDXKA_VERSION_HEADER}"
        VERBATIM
    )
    unset(_version_h_path)
# for single-config, we can use standard configure_file
else()
    configure_file(
        "${PDXKA_INCLUDE}/${PDXKA_VERSION_HEADER}.in"
        "include/${PDXKA_VERSION_HEADER}"
        @ONLY NEWLINE_STYLE LF
    )
    message(STATUS "Generated ${PDXKA_VERSION_HEADER}")
endif()

# test or other data directory root
set(PDXKA_DATA_DIR ${CMAKE_CURRENT_SOURCE_DIR}/data)

add_subdirectory(src)

if(BUILD_TESTS)
    # generate path.hh header for test paths
    # FIXME: same issue that version.h used to have, which is that WSL/Windows
    # has different results so builds will affect each other and that with
    # multi-config generators the PDXKA_BINARY_DIR is inaccurate
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
