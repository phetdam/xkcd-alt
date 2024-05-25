cmake_minimum_required(VERSION ${CMAKE_MINIMUM_REQUIRED_VERSION})

# unit test runner using Boost.Test
add_executable(pdxka_test features_test.cc main.cc version_test.cc)
target_link_libraries(
    pdxka_test PRIVATE
    Boost::filesystem Boost::unit_test_framework
)

add_test(NAME pdxka_test COMMAND pdxka_test)
