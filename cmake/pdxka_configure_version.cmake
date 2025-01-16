cmake_minimum_required(VERSION 3.13)

##
# pdxka_configure_version.cmake
#
# Run configure_file on version.h.in as a pre-build step in order to allow
# separate version.h files to be generated per build config for multi-config
# generators. This ensures that the build config listed in the header is
# accurate in the case of multi-config generators.
#
# External variables consumed:
#
#   PDXKA_VERSION_HEADER_IN     Path to the version.h.in input in source tree
#   PDXKA_VERSION_HEADER        Path to the version.h output to generate
#   PDXKA_MAJOR_VERSION
#   PDXKA_MINOR_VERSION
#   PDXKA_PATCH_VERSION
#   PDXKA_VERSION
#   PDXKA_BUILD_TYPE
#   PDXKA_SYSTEM_NAME
#   PDXKA_SYSTEM_VERSION
#   PDXKA_SYSTEM_ARCH
#   PDXKA_PROGNAME
#   Boost_VERSION_MACRO
#   Boost_VERSION_STRING
#   Boost_MAJOR_VERSION
#   Boost_MINOR_VERSION
#   Boost_SUBMINOR_VERSION
#   CURL_VERSION_STRING
#

##
# Check that the given variable is defined and not empty.
#
# Arguments:
#   var     CMake variable name
function(pdxka_check_var var)
    if(NOT DEFINED ${var})
        message(FATAL_ERROR "${var} is not defined")
    endif()
    # don't use NOT because if the value is zero the if will succeed
    if(${var} STREQUAL "")
        message(FATAL_ERROR "${var} is the empty string")
    endif()
endfunction()

# execute only in CMake script mode
if(CMAKE_SCRIPT_MODE_FILE)
    # check to ensure variables are defined
    pdxka_check_var(PDXKA_VERSION_HEADER_IN)
    pdxka_check_var(PDXKA_VERSION_HEADER)
    pdxka_check_var(PDXKA_MAJOR_VERSION)
    pdxka_check_var(PDXKA_MINOR_VERSION)
    pdxka_check_var(PDXKA_PATCH_VERSION)
    pdxka_check_var(PDXKA_VERSION)
    # TODO: add more checks
    # configure using consumed variables
    configure_file(
        ${PDXKA_VERSION_HEADER_IN} ${PDXKA_VERSION_HEADER}
        @ONLY NEWLINE_STYLE LF
    )
endif()
