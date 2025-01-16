cmake_minimum_required(VERSION ${CMAKE_MINIMUM_REQUIRED_VERSION})

##
# pdxka_test_helpers.cmake
#
# This module provides CTest test helpers, in particular a CMake function to
# query a Boost.Test program's list of tests.
#

##
# Add each active test in a Boost.Test program to the list of CTest tests.
#
# Arguments:
#   target
#       Boost.Test unit test runner
#
function(pdxka_boost_discover_tests target)
    # prefix for the target's CMake script for adding the Boost tests
    set(ctest_add_test_prefix ${CMAKE_BINARY_DIR}/add_${target}_boost_tests)
    # run CMake in script mode as a post-build command to add tests. note that
    # for multi-config generators the config is used, while single-config has
    # "impl" instead used as the determining suffix
    add_custom_command(
        TARGET ${target} POST_BUILD
        COMMAND
            ${CMAKE_COMMAND}
                -DBOOST_TEST_TARGET=${target}
                -DBOOST_TEST_TARGET_PATH=$<TARGET_FILE:${target}>
                -DBOOST_TEST_ADD_FILE=${ctest_add_test_prefix}-$<IF:${PDXKA_IS_MULTI_CONFIG},$<CONFIG>,impl>.cmake
                -P "${PDXKA_CMAKE_MODULE_DIR}/pdxka_boost_discover_tests_impl.cmake"
        WORKING_DIRECTORY $<TARGET_FILE_DIR:${target}>
        VERBATIM
    )
    # write the CTest include file
    set(ctest_include_file ${ctest_add_test_prefix}.cmake)
    if(PDXKA_IS_MULTI_CONFIG)
        file(
            WRITE "${ctest_include_file}"
            # multi-config generators require -C option to be used with CTest.
            # if not provided, simply raise a warning instead of failing
            "if(NOT CTEST_CONFIGURATION_TYPE)\n"
            "    message(\n"
            "        WARNING\n"
            "        \"No -C <config> specified for multi-config generator \"\n"
            "\"${CMAKE_GENERATOR}. ${target} tests will not be run.\"\n"
            ")\n"
            # configuration specified
            "else()\n"
            "    include(\"${ctest_add_test_prefix}-\${CTEST_CONFIGURATION_TYPE}.cmake\")\n"
            "endif()\n"
        )
    else()
        file(
            WRITE "${ctest_include_file}"
            "include(\"${ctest_add_test_prefix}-impl.cmake\")"
        )
    endif()
    # add discovered tests to test include files
    set_property(
        DIRECTORY APPEND PROPERTY
        TEST_INCLUDE_FILES "${ctest_include_file}"
    )
endfunction()
