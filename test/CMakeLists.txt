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
    add_custom_command(
        TARGET pdxka_test POST_BUILD
        COMMAND
            ${CMAKE_COMMAND} -E copy_if_different
                $<TARGET_RUNTIME_DLLS:pdxka_test> $<TARGET_FILE_DIR:pdxka_test>
        COMMENT "Copying dependent DLLs for pdxka_test"
        COMMAND_EXPAND_LISTS
        VERBATIM
    )
endif()

add_test(NAME pdxka_test COMMAND pdxka_test)

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
