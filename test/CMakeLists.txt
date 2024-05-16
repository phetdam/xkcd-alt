cmake_minimum_required(VERSION ${CMAKE_MINIMUM_REQUIRED_VERSION})

add_executable(pdxka_bpo_check pdxka_bpo_check.cc)
target_link_libraries(pdxka_bpo_check PRIVATE Boost::filesystem Boost::timer)

# FIXME: currently use cmake -E env to wrap the test invocation so we can skip
# the test even though it throws an exception using SKIP_RETURN_CODE
add_test(
    NAME pdxka_bpo_check
    COMMAND ${CMAKE_COMMAND} -E env $<TARGET_FILE:pdxka_bpo_check>
)
set_tests_properties(pdxka_bpo_check PROPERTIES SKIP_RETURN_CODE 1)
