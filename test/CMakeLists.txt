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
    # note: need compilation with /permissive- because otherwise it appears as
    # if when the program_main() overload in testing/program_main.hh takes an
    # prvalue as the argv argument_vector, the argv.argv() (decays to char**)
    # seems to return a pointer to an array of garbage despite the actual
    # argument_vector tuple members being correctly set. this may be due to
    # /Zc:rvalueCast being off by default. it also shows, however, that we need
    # to have a proper copy ctor for argument_vector; the argv pointers will
    # get invalidated if a copy is made and the original is discarded.
    # TODO: remove /permissive- once we have a better copy ctor
    target_compile_options(pdxka_test PRIVATE /permissive-)
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
