cmake_minimum_required(VERSION ${CMAKE_MINIMUM_REQUIRED_VERSION})

# unit test runner using Boost.Test
add_executable(
    pdxka_test
    curl_test.cc features_test.cc main.cc program_main_test.cc version_test.cc
)
# TODO: on Windows, need to copy the CURL DLL
target_link_libraries(
    pdxka_test PRIVATE
    Boost::filesystem Boost::unit_test_framework CURL::libcurl pdxka
)
# on Windows we need to define NOMINMAX since the Windows min() and max()
# macros are somehow leaking in testing/program_main.hh
if(WIN32)
    target_compile_definitions(pdxka_test PRIVATE NOMINMAX)
endif()

add_test(NAME pdxka_test COMMAND pdxka_test)
