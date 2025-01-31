cmake_minimum_required(VERSION 3.20)

project(
    XkcdAltCMakeUsage
    VERSION 0.1.0
    DESCRIPTION "xkcd-alt CMake usage test"
    LANGUAGES CXX
)

##
# Check if a variable is defined and not false.
#
# Arguments:
#   var     Variable name
#
function(check_var var)
    if(NOT DEFINED ${var})
        message(FATAL_ERROR "${var} not defined")
    endif()
    if(NOT ${var})
        message(FATAL_ERROR "${var} (${${var}}) evaluates to false in if")
    endif()
endfunction()

# check + print root
check_var(XkcdAlt_ROOT)
check_var(PDXKA_FIND_VERSION)
message(STATUS "XkcdAlt_ROOT: ${XkcdAlt_ROOT}")

# find package
find_package(XkcdAlt ${PDXKA_FIND_VERSION} REQUIRED)
# if not found, error (if found, should be true)
check_var(XkcdAlt_FOUND)
# should have version that is compatible
check_var(XkcdAlt_VERSION)
if(XkcdAlt_VERSION VERSION_GREATER PDXKA_FIND_VERSION)
    message(
        FATAL_ERROR
        "XkcdAlt_VERSION ${XkcdAlt_VERSION} > ${PDXKA_FIND_VERSION}"
    )
endif()
# print version info
message(STATUS "XkcdAlt version: ${XkcdAlt_VERSION}")

# check that the executable exists (could use IMPORTED_LOCATION)
get_target_property(xkcd_alt_path XkcdAlt::xkcd-alt LOCATION)
if(NOT EXISTS ${xkcd_alt_path})
    message(FATAL_ERROR "xkcd-alt target path ${xkcd_alt_path} doesn't exist")
endif()
