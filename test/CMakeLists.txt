cmake_minimum_required(VERSION ${CMAKE_MINIMUM_REQUIRED_VERSION})

# pdxka_test: Boost.Test unit test runner
# note: see src/CMakeLists.txt for CTest tests that use regex patterns to check
# the output of xkcd-alt. previously, we used Boost.Process to launch xkcd-alt
# with the -V option to print version info, which we would then capture and
# then check in C++ to see if the Boost/libcurl version info was correct. the
# CTest tests provide an easier and more direct way to perform these tests.
add_executable(pdxka_test curl_test.cc main.cc program_main_test.cc)
target_link_libraries(
    pdxka_test PRIVATE
    Boost::filesystem Boost::unit_test_framework CURL::libcurl pdxka
)
# if multi-config, also need to use per-config testing/path.hh config step
if(PDXKA_IS_MULTI_CONFIG)
    add_dependencies(pdxka_test pdxka_testing_path_hh)
endif()
# Windows-specific settings for pdxka_test
if(WIN32)
    # com.hh tests included
    target_sources(pdxka_test PRIVATE com_test.cc)
    # on Windows we need to define NOMINMAX since the Windows min() and max()
    # macros are somehow leaking in testing/program_main.hh
    # note: can get rid of the _WIN32_WINNT not defined Boost.Asio warning by
    # compiling with /DBOOST_USE_WINDOWS_H and /DWIN32_LEAN_AND_MEAN (latter is
    # required is avoid including Windows Sockets 1 in Boost.Asio header), but
    # this seems like a pretty heavy workaround (including Windows.h). other
    # option is to use /DWIN32_WINNT=0x0601 (_WIN32_WINNT_WIN7 in sdkddkver.h)
    # but it doesn't seem great to make this a project-wide define either.
    target_compile_definitions(
        pdxka_test PRIVATE
        NOMINMAX BOOST_USE_WINDOWS_H WIN32_LEAN_AND_MEAN
    )
    # disable C5204 emitted from pplwin.h (probably because this header is used
    # by Boost.ProgramOptions; not entirely sure however)
    target_compile_options(pdxka_test PRIVATE /wd5204)
    #
    # note:
    #
    # although we fixed the issue with the testing/program_main.hh
    # argument_vector not having the proper copy semantics (see copy ctor
    # comments for details), without /permissive- Visual Studio 2022 calls the
    # copy ctor for some reason when passing a prvalue argument_vector to a
    # function template that is taking an argument_vector rvalue reference.
    # this is probably related to MSVC's default behavior when /Zc:rvalueCast
    # is not set but in this context there is no cast operation being done.
    #
    # note:
    #
    # some flakiness of pdkxa_test Boost.PropertyTree errors when we change
    # this? seems like part of the build is not getting triggered
    #
    # target_compile_options(pdxka_test PRIVATE /permissive-)
    # com_test.cc uses IWICImagingFactory and WindowsCodecs is needed for the
    # definition of the extern IID_IWICImagingFactory
    target_link_libraries(pdxka_test PRIVATE ole32 WindowsCodecs)
    # copy dependent DLLs to output directory
    pdxka_copy_runtime_dlls(pdxka_test)
endif()

# add Boost tests individually
pdxka_boost_discover_tests(pdxka_test)

# pdxka_inet_test: check if we are connected to the Internet
# this is used by pdxka_live_test.cmake to skip live xkcd-alt tests if there is
# no Internet connection. we cannot use test fixtures because if a fixture test
# is skipped, the dependent tests will still get fired off.
add_executable(pdxka_inet_test pdxka_inet_test.cc)
if(WIN32)
    target_link_libraries(pdxka_inet_test PRIVATE ole32)
endif()

##
# Adds a CTest test that runs a test of pdxka_find_curl.
#
# This is wrapped in a CMake script that will run pdxka_find_curl to locate
# CURL. The command-line arguments passed to pdxka_find_curl_test.cmake that is
# run in CMake script mode are used to control the arguments passed to
# pdxka_find_curl; see pdxka_find_curl_test.cmake for usage.
#
# Arguments:
#   ID id                   Test ID suffix. This is joined with the test prefix
#                           so the final name is be "pdxka_find_curl_test_<id>"
#   ARGS ...                Arguments to pass to pdxka_find_curl_test.cmake
#   PASS_REGEX <regex>      Regular expression to set as the for the test's
#                           PASS_REGULAR_EXPRESSION property
#
function(add_find_curl_test)
    # parse arguments
    cmake_parse_arguments(_ARG "" "ID;PASS_REGEX" "ARGS" ${ARGN})
    # since CURL::libcurl is defined extract the install root
    get_target_property(_curl_root CURL::libcurl IMPORTED_LOCATION)
    # note: assumes we only need to go up one directory level
    cmake_path(GET _curl_root PARENT_PATH _curl_root)
    cmake_path(GET _curl_root PARENT_PATH _curl_root)
    # register test
    add_test(
        NAME pdxka_find_curl_test_${_ARG_ID}
        COMMAND
            ${CMAKE_COMMAND}
                -DCURL_ROOT=${_curl_root}
                -P ${CMAKE_CURRENT_SOURCE_DIR}/pdxka_find_curl_test.cmake --
                -m ${PDXKA_SOURCE_DIR}/cmake/pdxka_find_curl.cmake ${_ARG_ARGS}
    )
    set_property(
        TEST pdxka_find_curl_test_${_ARG_ID} PROPERTY
        PASS_REGULAR_EXPRESSION "${_ARG_PASS_REGEX}"
    )
endfunction()

# tests for pdxka_find_curl
# 1. optional search, version too high
add_find_curl_test(ID 1 ARGS -v 1900.90.0 PASS_REGEX "requested version")
# 2. required search, version too high
add_find_curl_test(ID 2 ARGS -r -v 900.0.1 PASS_REGEX "required version")
# 3. optional search, version is ok, unknown component
add_find_curl_test(
    ID 3
    # note: HTTPS is required anyways for this project
    ARGS -v ${PDXKA_CURL_VERSION} -c HTTPS,invalid_component
    PASS_REGEX "Could NOT find requested libcurl component invalid_component"
)

# path to test install directory
set(_test_install_dir ${PDXKA_BINARY_DIR}/test_install)

# helper variable
set(_prog ${PDXKA_PROGNAME})
# pre-install test to remove the previous install directory
# note: for multi-config generatores, this overwrites the executable
add_test(
    NAME ${_prog}_prepare_install_test
    COMMAND ${CMAKE_COMMAND} -E rm -rf ${_test_install_dir}
)
# sets up the test_install_clean fixture
set_tests_properties(
    ${_prog}_prepare_install_test PROPERTIES
    FIXTURES_SETUP test_install_clean
)

# test project installation
# multi-config needs --config argument
if(PDXKA_IS_MULTI_CONFIG)
    add_test(
        NAME ${_prog}_install_test
        COMMAND ${CMAKE_COMMAND}
                --install ${PDXKA_BINARY_DIR} --prefix ${_test_install_dir}
                --config $<CONFIG>
    )
else()
    add_test(
        NAME ${_prog}_install_test
        COMMAND ${CMAKE_COMMAND}
                --install ${PDXKA_BINARY_DIR} --prefix ${_test_install_dir}
    )
endif()
# requires clean step already run + set up test_install fixture
set_tests_properties(
    ${_prog}_install_test PROPERTIES
    FIXTURES_SETUP test_install
    FIXTURES_REQUIRED test_install_clean
)

# path to test CMake build directory
set(_test_cmake_dir ${PDXKA_BINARY_DIR}/test_cmake_usage)

# pre-CMake usage test to remove the previous CMake build dirctory
add_test(
    NAME ${_prog}_prepare_cmake_usage
    COMMAND ${CMAKE_COMMAND} -E rm -rf ${_test_cmake_dir}
)
# sets up cmake_usage_clean fixture (note: can be done in parallel to install)
set_tests_properties(
    ${_prog}_prepare_cmake_usage PROPERTIES
    FIXTURES_SETUP test_cmake_usage_clean
)

# test CMake configuration step
add_test(
    NAME ${_prog}_cmake_config_test
    COMMAND ${CMAKE_COMMAND}
            -S ${CMAKE_CURRENT_SOURCE_DIR}/pdxka_cmake_usage_test
            -B ${_test_cmake_dir}
            -DXkcdAlt_ROOT=${_test_install_dir}
            -DPDXKA_FIND_VERSION=${PROJECT_VERSION}
)
# requires install + clean CMake build directory + sets up config fixture
set_tests_properties(
    ${_prog}_cmake_config_test PROPERTIES
    FIXTURES_SETUP test_cmake_config
    FIXTURES_REQUIRED "test_install;test_cmake_usage_clean"
)

unset(_prog)
