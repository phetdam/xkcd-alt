cmake_minimum_required(VERSION ${CMAKE_MINIMUM_REQUIRED_VERSION})

# check that reporting of whether or not Boost.ProgramOptions is used at
# runtime matches the value of the compile-time feature
add_executable(pdxka_bpo_check pdxka_bpo_check.cc)
target_link_libraries(pdxka_bpo_check PRIVATE Boost::filesystem Boost::timer)

add_test(NAME pdxka_bpo_check COMMAND pdxka_bpo_check)
