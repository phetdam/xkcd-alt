cmake_minimum_required(VERSION 3.13)

##
# pdxka_boost_discover_tests_impl.cmake
#
# This module provides the implementation function that is run in CMake script
# mode post-build to generate the list of tests for a Boost.Test runner. The
# implementation is inspired by that of gtest_discover_tests, borrowing some
# of the same techniques, but with a simpler implementation.
#

##
# Generate the CMake script that adds the Boost tests.
#
# This script will be ingested by CMake. See pdxka_boost_discover_tests.
#
# External variables are read by this function:
#
#   BOOST_TEST_TARGET           Name of the test target
#   BOOST_TEST_TARGET_PATH      Full path to the test target
#   BOOST_TEST_ADD_FILE         Path to the final CMake script path name
#
function(pdxka_boost_discover_tests_impl)
    # execute to get the list of tests
    execute_process(
        # path should be target's full path
        COMMAND ${BOOST_TEST_TARGET_PATH} --list_content
        # note: timeout currently hardcoded
        TIMEOUT 15
        RESULT_VARIABLE _list_content_result
        # --list_content writes to stderr for some reason
        ERROR_VARIABLE _list_content_output
        ERROR_STRIP_TRAILING_WHITESPACE
    )
    # failed
    if(_list_content_result)
        message(
            WARNING
            "Unable to get list of tests with ${BOOST_TEST_TARGET} --list-content"
        )
        return()
    endif()
    # otherwise, success. get the content output as list of lines
    string(REPLACE "\n" ";" _list_content_lines "${_list_content_output}")
    # last test indentation level. -1 indicates first test
    set(_last_indent -1)
    # last test filter to add. this will have a leading "/" for convenience
    set(_last_test "")
    # add_test commands that CTest should run. we cannot use a CMake list
    # because some of the test names contain spaces (confuses CMake)
    set(ctest_test_script "")
    # loop through the lines
    foreach(_line ${_list_content_lines})
        # determine indentation level of the line by stripping leading indents
        string(REPLACE "    " "" _stripped_line "${_line}")
        string(LENGTH ${_line} _line_len)
        string(LENGTH ${_stripped_line} _stripped_line_len)
        math(EXPR _cur_indent "${_line_len} - ${_stripped_line_len}")
        # if the last character is not *, the test is inactive, so skip
        math(EXPR _stripped_line_last_i "${_stripped_line_len} - 1")
        string(SUBSTRING "${_stripped_line}" ${_stripped_line_last_i} 1 _active)
        if(NOT _active STREQUAL "*")
            continue()
        endif()
        # otherwise, remove the trailing star
        string(SUBSTRING "${_stripped_line}" 0 ${_stripped_line_last_i} _stripped_line)
        # if current indent > last indent, add another test component
        if(_cur_indent GREATER _last_indent)
            string(APPEND _last_test "/${_stripped_line}")
        # otherwise, we need to add the test pattern
        else()
            # need to remove the leading "/" from the test pattern + append
            string(SUBSTRING "${_last_test}" 1 -1 _last_test_filter)
            string(
                APPEND ctest_test_script
                # note: have to use old-style add_test
                "add_test(\n"
                "    \"${BOOST_TEST_TARGET}/${_last_test_filter}\"\n"
                "    ${BOOST_TEST_TARGET_PATH} -t \"${_last_test_filter}\"\n"
                # extra newline necessary
                ")\n"
            )
            # DEBUG
            # message(STATUS "test: ${BOOST_TEST_TARGET}${_last_test}")
            # if current indent < last indent, need to back up indent levels
            if(_cur_indent LESS _last_indent)
                # compute the number of indentation levels (4 spaces per indent)
                math(EXPR _indent_diff "(${_last_indent} - ${_cur_indent}) / 4")
                # remove the last test filter component _indent_diff - 1 times.
                # _indent_diff is guaranteed to be at least 1
                foreach(_i RANGE 1 ${_indent_diff})
                    string(REGEX REPLACE "/[^/]+$" "" _last_test "${_last_test}")
                endforeach()
            endif()
            # replace last test filter component with the new stripped line
            string(
                REGEX REPLACE
                "/[^/]+$" "/${_stripped_line}" _last_test "${_last_test}"
            )
        endif()
        # update last indent
        set(_last_indent ${_cur_indent})
    endforeach()
    # append the last test pattern
    string(SUBSTRING "${_last_test}" 1 -1 _last_test_filter)
    string(
        APPEND ctest_test_script
        "add_test(\n"
        "    \"${BOOST_TEST_TARGET}/${_last_test_filter}\"\n"
        "    ${BOOST_TEST_TARGET_PATH} -t \"${_last_test_filter}\"\n"
        ")\n"
    )
    # DEBUG
    # message(STATUS "test: ${BOOST_TEST_TARGET}${_last_test}")
    # message(STATUS "script: ${ctest_test_script}")
    # write the test patterns to BOOST_TEST_ADD_FILE
    file(WRITE ${BOOST_TEST_ADD_FILE} "${ctest_test_script}")
endfunction()

# execute only in CMake script mode
if(CMAKE_SCRIPT_MODE_FILE)
    pdxka_boost_discover_tests_impl()
endif()
