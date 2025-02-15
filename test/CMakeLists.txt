cmake_minimum_required(VERSION ${CMAKE_MINIMUM_REQUIRED_VERSION})

# unit test runner using Boost.Test
# TODO: for tests that check the output xkcd-alt it might be better to make
# them simple CTest tests that we will check the output regex for pass/fail.
# the version macros checked are known to CMake (since configure_file is used
# to generate the headers) so testing inside C++ may be unnecessary. see the
# add_test() commands in ../src where we test xkcd-alt -V, etc.
add_executable(
    pdxka_test
    curl_test.cc features_test.cc main.cc program_main_test.cc version_test.cc
)
target_link_libraries(
    pdxka_test PRIVATE
    Boost::filesystem Boost::unit_test_framework CURL::libcurl pdxka
)
# if multi-config, also need to use per-config testing/path.hh config step
if(PDXKA_IS_MULTI_CONFIG)
    add_dependencies(pdxka_test pdxka_testing_path_hh)
endif()
# on Windows we need to define NOMINMAX since the Windows min() and max()
# macros are somehow leaking in testing/program_main.hh
# note: can get rid of the _WIN32_WINNT not being defined Boost.Asio warning by
# compiling with /DBOOST_USE_WINDOWS_H and /DWIN32_LEAN_AND_MEAN (latter is
# required is avoid including Windows Sockets 1 in Boost.Asio header), but this
# seems like a pretty heavy workaround (including Windows.h). the other option
# is to use /DWIN32_WINNT=0x0601 (_WIN32_WINNT_WIN7 in sdkddkver.h) but it
# doesn't seem great to make this a project-wide define either.
if(WIN32)
    target_compile_definitions(
        pdxka_test PRIVATE
        NOMINMAX BOOST_USE_WINDOWS_H WIN32_LEAN_AND_MEAN
    )
    # disable C5204 emitted from pplwin.h (probably because this header is used
    # by Boost.ProgramOptions; not entirely sure however)
    target_compile_options(pdxka_test PRIVATE /wd5204)
    # note: although we fixed the issue with the testing/program_main.hh
    # argument_vector not having the proper copy semantics (see copy ctor
    # comments for details), without /permissive- Visual Studio 2022 calls the
    # copy ctor for some reason when passing a prvalue argument_vector to a
    # function template that is taking an argument_vector rvalue reference.
    # this is probably related to MSVC's default behavior when /Zc:rvalueCast
    # is not set but in this context there is no cast operation being done.
    # note: some flakiness of pdkxa_test Boost.PropertyTree errors when we
    # change this? seems like part of the build is not getting triggered
    # target_compile_options(pdxka_test PRIVATE /permissive-)
    # copy dependent DLLs to output directory
    pdxka_copy_runtime_dlls(pdxka_test)
endif()

# add Boost tests individually
pdxka_boost_discover_tests(pdxka_test)

# tests for pdxka_find_curl
# 1. optional search, version too high
add_test(
    NAME pdxka_find_curl_test_1
    COMMAND
        ${CMAKE_COMMAND} -P
            ${CMAKE_CURRENT_SOURCE_DIR}/pdxka_find_curl_test.cmake
            --
            # TODO: should have a helper variable for this path
            -m ${CMAKE_CURRENT_SOURCE_DIR}/../cmake/pdxka_find_curl.cmake
            -v 1900.90.0
)
set_property(
    TEST pdxka_find_curl_test_1 PROPERTY
    PASS_REGULAR_EXPRESSION "requested version"
)
# 2. required search, version too high
add_test(
    NAME pdxka_find_curl_test_2
    COMMAND
        ${CMAKE_COMMAND} -P
        ${CMAKE_CURRENT_SOURCE_DIR}/pdxka_find_curl_test.cmake
        --
        -m ${CMAKE_CURRENT_SOURCE_DIR}/../cmake/pdxka_find_curl.cmake
        -r
        -v 900.0.1
)
set_property(
    TEST pdxka_find_curl_test_2 PROPERTY
    PASS_REGULAR_EXPRESSION "required version"
)
# 3. optional search, version is ok, unknown component
add_test(
    NAME pdxka_find_curl_test_3
    COMMAND
        ${CMAKE_COMMAND} -P
        ${CMAKE_CURRENT_SOURCE_DIR}/pdxka_find_curl_test.cmake
        --
        -m ${CMAKE_CURRENT_SOURCE_DIR}/../cmake/pdxka_find_curl.cmake
        -v  ${PDXKA_CURL_VERSION}
        # note: HTTPS is required anyways for this project
        -c "HTTPS;nonexistent_libcurl_component"
)
set_property(
    TEST pdxka_find_curl_test_3 PROPERTY
    PASS_REGULAR_EXPRESSION "Could NOT find requested libcurl component"
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
    FIXTURES_REQUIRED "test_install;tset_cmake_usage_clean"
)

unset(_prog)
