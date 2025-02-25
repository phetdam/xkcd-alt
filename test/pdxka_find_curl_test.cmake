cmake_minimum_required(VERSION 3.20)

##
# Test driver script for the pdxka_find_curl() macro.
#
# This is intended to be invoked via cmake -P (script mode) as follows:
#
#   cmake -P pdxka_find_curl_test.cmake --
#       -m /path/to/pdxka_find_curl.cmake [-v VERSION] [-r] [-c COMPONENTS]
#
# Usually this should be invoked via CTest by the project itself.
#

# usage string
set(usage "Help text:\n\nUsage: cmake -P pdxka_find_curl_test.cmake --\n")
string(
    APPEND usage
    "    -m /path/to/pdxka_find_curl.cmake [-v VERSION] [-r] [-c COMPONENTS]\n\n"
)
string(APPEND usage "Perform tests on pdxka_find_curl.\n\n")
string(
    APPEND usage
    "If running standalone you may need to define CURL_ROOT as either an\n"
)
string(
    APPEND usage
    "environment variable or as a CMake variable with -DCURL_ROOT=<path>.\n\n"
)
string(APPEND usage "Options:\n")
string(APPEND usage "  -h, --help        Print this usage text\n")
string(APPEND usage "  -m MODULE, --module MODULE\n")
string(APPEND usage "                    Required path to pdxka_find_curl.cmake\n\n")
string(APPEND usage "  -v VERSION, --version VERSION\n")
string(
    APPEND usage
    "                    Optional libcurl minimum required version\n\n"
)
string(APPEND usage "  -r, --required    Specify if libcurl search must succeed\n\n")
string(APPEND usage "  -c COMPONENTS, --components COMPONENTS\n")
string(
    APPEND usage
    "                    Optional comma-separated libcurl components to\n"
    "                    search for as one would use with FindCURL"
)

##
# Parse the incoming command-line arguments.
#
# The following variables are set on successful completion:
#
#   arg_list        List of CMake arguments to pass to pdxka_find_curl
#
macro(parse_args)
    # loop through incoming args and skip until we hit "--"
    foreach(_i RANGE 0 ${CMAKE_ARGC})
        if(CMAKE_ARGV${_i} STREQUAL "--")
            set(first_arg_i ${_i})
            break()
        endif()
    endforeach()
    # if first_arg_i is not defined, we are missing "--"
    if(NOT DEFINED first_arg_i)
        message(STATUS "${usage}")
        message(FATAL_ERROR "Missing -- in argument list")
    endif()
    # argument list
    set(arg_list "")
    # loop through relevant args
    foreach(_i RANGE ${first_arg_i} ${CMAKE_ARGC})
        # skip first arg
        if(_i EQUAL first_arg_i)
            continue()
        endif()
        # build argument list
        # -h, --help
        if(CMAKE_ARGV${_i} STREQUAL "-h" OR CMAKE_ARGV${_i} STREQUAL "--help")
            message(STATUS "${usage}")
            # terminates CMake script mode
            return()
        # -m, --module
        elseif(
            CMAKE_ARGV${_i} STREQUAL "-m" OR
            CMAKE_ARGV${_i} STREQUAL "--module"
        )
            # error if already specified
            if(DEFINED module_included)
                message(FATAL_ERROR "-m, --module already specified")
            endif()
            # get index of next arg
            # note: this does *not* change _i in the foreach scope!
            math(EXPR _i "${_i} + 1")
            # must be < argc
            if(_i GREATER_EQUAL CMAKE_ARGC)
                message(FATAL_ERROR "-m, --module missing required version string")
            endif()
            # include the module
            include(${CMAKE_ARGV${_i}})
            set(module_included TRUE)  # indicator
        # -r, --required
        elseif(
            CMAKE_ARGV${_i} STREQUAL "-r" OR
            CMAKE_ARGV${_i} STREQUAL "--required"
        )
            # error if already specified
            if(DEFINED required_added)
                message(FATAL_ERROR "-r, --required already specified")
            endif()
            list(APPEND arg_list "REQUIRED")
            set(required_added TRUE)  # indicator
        # -v, --version
        elseif(
            CMAKE_ARGV${_i} STREQUAL "-v" OR
            CMAKE_ARGV${_i} STREQUAL "--version"
        )
            # error if already specified
            if(DEFINED version_added)
                message(FATAL_ERROR "-v, --version already specified")
            endif()
            # get index of next arg
            math(EXPR _i "${_i} + 1")
            # must be < argc
            if(_i GREATER_EQUAL CMAKE_ARGC)
                message(FATAL_ERROR "-v, --version missing required version string")
            endif()
            # add the version argument (unchecked)
            list(APPEND arg_list "VERSION" "${CMAKE_ARGV${_i}}")
            set(version_added TRUE)  # indicator
        # -c, --components
        elseif(
            CMAKE_ARGV${_i} STREQUAL "-c" OR
            CMAKE_ARGV${_i} STREQUAL "--components"
        )
            # error if already specified
            if(DEFINED components_added)
                message(FATAL_ERROR "-c, --components already specified")
            endif()
            # get index of next arg
            math(EXPR _i "${_i} + 1")
            # must be < argc
            if(_i GREATER_EQUAL CMAKE_ARGC)
                message(FATAL_ERROR "-c, --components missing required arguments")
            endif()
            # convert into a list
            set(_components "${CMAKE_ARGV${_i}}")
            string(REPLACE "," ";" _components "${_components}")
            # add the components list
            list(APPEND arg_list "COMPONENTS" ${_components})
            # set indiicator and unset unused
            set(components_added TRUE)
            unset(_components)
        endif()
    endforeach()
    # if module is not included, error
    if(NOT module_included)
        message(FATAL_ERROR "No -m, --module specified to include")
    endif()
    # unset indicators
    unset(module_included)
    unset(required_added)
    unset(version_added)
    unset(components_added)
endmacro()

# parse arguments and set relevant variables
parse_args()
# print call signature
list(JOIN arg_list " " arg_print_list)
message(STATUS "Calling pdxka_find_curl(${arg_print_list})")
# call macro
pdxka_find_curl(${arg_list})
