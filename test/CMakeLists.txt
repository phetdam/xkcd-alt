cmake_minimum_required(VERSION ${CMAKE_MINIMUM_REQUIRED_VERSION})

# check that reporting of whether or not Boost.ProgramOptions is used at
# runtime matches the value of the compile-time feature
add_executable(pdxka_bpo_check pdxka_bpo_check.cc)
target_link_libraries(pdxka_bpo_check PRIVATE Boost::filesystem Boost::timer)
# if using Boost.Test to implement the tests
if(PDXKA_BPO_CHECK_USE_BOOST_TEST)
    target_compile_definitions(
        pdxka_bpo_check
        PRIVATE PDXKA_BPO_CHECK_USE_BOOST_TEST
    )
    target_link_libraries(pdxka_bpo_check PRIVATE Boost::unit_test_framework)
endif()

add_executable(pdxka_test features_test.cc main.cc version_test.cc)
target_link_libraries(
    pdxka_test PRIVATE
    Boost::filesystem Boost::unit_test_framework
)

add_test(NAME pdxka_bpo_check COMMAND pdxka_bpo_check)
add_test(NAME pdxka_test COMMAND pdxka_test)
