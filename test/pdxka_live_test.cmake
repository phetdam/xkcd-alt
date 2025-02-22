cmake_minimum_required(VERSION 3.21)

##
# pdxka_live_test.cmake
#
# This CMake script runs the xkcd-alt program, network connectivity and all, to
# make requests to the XKCD RSS feed. However, if there is not network
# connection, as determined by pdxka_inet_test, the test is skipped.
#

# only run in CMake script mode
if(NOT CMAKE_SCRIPT_MODE_FILE)
    message(FATAL_ERROR "This CMake module can only be run in script mode")
endif()

##
# Checks that a variable is defined.
#
# Arguments:
#   var     Variable name
#
function(check_var var)
    if(NOT DEFINED ${var})
        message(FATAL_ERROR "Variable ${var} is not defined")
    endif()
endfunction()

# check the following external variables:
#
#   PDXKA_INET_TEST     Path to the pdxka_inet_test program
#   XKCD_ALT_PROGRAM    Path to the xkcd-alt program to run
#
# command-line arguments passed to xkcd-alt are read from CMAKE_ARGV[N].
#
check_var(PDXKA_INET_TEST)
check_var(XKCD_ALT_PROGRAM)

# find index of -- which is passed to separate user arguments from CMake ones
foreach(_i RANGE ${CMAKE_ARGC})
    if(CMAKE_ARGV${_i} STREQUAL "--")
        set(first_arg_i ${_i})
        break()
    endif()
endforeach()
if(NOT DEFINED first_arg_i)
    message(FATAL_ERROR "Missing -- in argument list")
endif()

# collect CMake arguments into a list
set(arg_list "")
foreach(_i RANGE ${first_arg_i} ${CMAKE_ARGC})
    # skip first argument
    if(_i EQUAL first_arg_i)
        continue()
    endif()
    # otherwise append into list
    list(APPEND arg_list "${CMAKE_ARGV${_i}}")
endforeach()
# for vanity purposes convert into space-separated string
list(JOIN arg_list " " arg_list_str)

# execute the connectivity check program
execute_process(
    COMMAND ${PDXKA_INET_TEST}
    # usually takes essentialy no time at all to run, but good to have
    TIMEOUT 10
    RESULT_VARIABLE inet_check_res
    OUTPUT_VARIABLE inet_check_output
    OUTPUT_STRIP_TRAILING_WHITESPACE
)
# if the output contains "Internet: No", then no internet, so print message to
# signal to CTest that the test is skipped and stop the script
string(FIND "${inet_check_output}" "Internet: No" no_inet_pos)
if(NOT no_inet_pos EQUAL -1)
    message(STATUS "SKIPPING: ${XKCD_ALT_PROGRAM} ${arg_list_str} (no Internet)")
    return()
endif()
# if the program failed for another reason, error
if(inet_check_res)
    message(
        FATAL_ERROR
        "${PDXKA_INET_TEST} failed with output:\n${inet_check_output}"
    )
endif()
# unset unused
unset(inet_check_res)
unset(inet_check_output)
unset(no_inet_pos)

# run the actual test command. we let CTest handle the rest afterwards
message(STATUS "Running ${XKCD_ALT_PROGRAM} ${arg_list_str}")
execute_process(
    COMMAND ${XKCD_ALT_PROGRAM} ${arg_list}
    RESULT_VARIABLE xkcd_alt_res
)
# error if failed
if(xkcd_alt_res)
    message(FATAL_ERROR "FAILED: ${XKCD_ALT_PROGRAN} ${arg_list_str}")
endif()
