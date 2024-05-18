cmake_minimum_required(VERSION ${CMAKE_MINIMUM_REQUIRED_VERSION})

add_executable(pdxka_bpo_check pdxka_bpo_check.cc)
target_link_libraries(pdxka_bpo_check PRIVATE Boost::filesystem Boost::timer)

add_test(NAME pdxka_bpo_check COMMAND pdxka_bpo_check)
