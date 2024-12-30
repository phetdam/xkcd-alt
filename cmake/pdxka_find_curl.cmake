cmake_minimum_required(VERSION 3.20)

##
# pdxka_find_curl.cmake
#
# This module provides a function to find CURL for Windows since the native
# provided CURLConfig.cmake was observed to have some issues.
#
# The main problem is that the CURLConfig.cmake script does *not* set
# CURL_FOUND to TRUE, so the CMake FindCURL.cmake find module always treats the
# config mode search for CURL as having failed. This can be seen if
# find_package(CURL CONFIG REQUIRED) is done (no find module wrapping).
#
# This could be fixed if CURLConfig.cmake sets CURL_FOUND to TRUE and then, for
# consistency with CMake's FindCURL, sets CURL_VERSION_STRING to CURL_VERSION.
# However, find_package(CURL) would still have to be done in CONFIG mode on
# Windows, as there is no component checking logic, and the FindCURL module
# *does* check if CURL_HTTPS_FOUND is set to TRUE, etc., for the requested
# components, via the find_package_handle_standard_args call.
#
# Therefore, we provide the pdxka_find_curl() macro that basically serves as a
# home-brew find module. We don't provide an actual FindCURL.cmake find module
# however as generally we still want to use CMake's implementation.
#

##
# Locate libcurl using the curl executable.
#
# On non-Windows systems a find_package(CURL) call should be used instead but
# this macro will still work. It's better to use the official FindCURL find
# module, however, if possible, as it will fall back to use pkg-config.
#
# When the macro completes, the following variables are defined:
#
#   CURL_FOUND              TRUE if all components found, FALSE otherwise
#
# If CURL_FOUND is true, then the following variables can be used:
#
#   CURL_INCLUDE_DIRS       Directory containing the curl/* headers
#   CURL_VERSION_MAJOR      libcurl major version component
#   CURL_VERSION_MINOR      libcurl minor version component
#   CURL_VERSION_PATCH      libcurl patch version component
#   CURL_VERSION            libcurl version string, e.g. 8.11.2-DEV
#   CURL_VERSION_STRING     libcurl version string, e.g. 8.11.2-DEV, provided
#                           for compatibility with CMake's FindCURL module
#
# The CURL::libcurl imported library target is also defined for use.
#
# Arguments:
#   VERSION version
#       Optional minimum required version to look for
#   REQUIRED
#       Control whether or not finding CURL is required
#   COMPONENTS components...
#       List of components to search for
#
macro(pdxka_find_curl)
    # on/off args
    set(_option_args REQUIRED)
    # single arg arguments
    set(_single_args VERSION)
    # multi-arg arguments
    set(_multi_args COMPONENTS)
    # parse arguments
    cmake_parse_arguments(
        _PDXKA_CURL
        "${_option_args}" "${_single_args}" "${_multi_args}" ${ARGN}
    )
    # clear arg lists
    unset(_option_args)
    unset(_single_args)
    unset(_multi_args)
    # build list of root hint locations. CURL_ROOT is preferred over CURL_DIR
    set(_curl_root_hints "")
    # check cache first
    if(DEFINED CACHE{CURL_ROOT})
        set(_curl_root_hints ${_curl_root_hints} $CACHE{CURL_ROOT})
    endif()
    if(DEFINED CACHE{CURL_DIR})
        set(_curl_root_hints ${_curl_root_hints} $CACHE{CURL_DIR})
    endif()
    # then check env
    if(DEFINED ENV{CURL_ROOT})
        set(_curl_root_hints ${_curl_root_hints} $ENV{CURL_ROOT})
    endif()
    if(DEFINED ENV{CURL_DIR})
        set(_curl_root_hints ${_curl_root_hints} $ENV{CURL_DIR})
    endif()
    # append program files
    if(WIN32)
        set(_curl_root_hints ${_curl_root_hints} "C:/Program Files")
    else()
        set(_curl_root_hints ${_curl_root_hints} "/usr/local")
    endif()
    # build include hints
    list(
        TRANSFORM _curl_root_hints
        APPEND "/include" OUTPUT_VARIABLE _curl_include_hints
    )
    # first set CURL_FOUND to true. this may be set to false later
    set(CURL_FOUND TRUE)
    # find required curl/curl.h
    find_file(PDXKA_CURL_H curl/curl.h HINTS ${_curl_include_hints} NO_CACHE)
    if(PDXKA_CURL_H STREQUAL "PDXKA_CURL_H-NOTFOUND")
        # error if required, otherwise not found
        if(DEFINED _PDXKA_CURL_REQUIRED)
            message(FATAL_ERROR "Unable to find curl/curl.h")
        else()
            set(CURL_FOUND FALSE)
        endif()
    endif()
    # set CURL_INCLUDE_DIRS
    cmake_path(CONVERT "${PDXKA_CURL_H}" TO_CMAKE_PATH_LIST CURL_INCLUDE_DIRS)
    # back up to include level
    set(CURL_INCLUDE_DIRS "${CURL_INCLUDE_DIRS}/../../")
    cmake_path(NORMAL_PATH CURL_INCLUDE_DIRS OUTPUT_VARIABLE CURL_INCLUDE_DIRS)
    # set the curl root
    set(CURL_INSTALL_ROOT "${CURL_INCLUDE_DIRS}/../")
    cmake_path(NORMAL_PATH CURL_INSTALL_ROOT OUTPUT_VARIABLE CURL_INSTALL_ROOT)
    # find required curl/curlver.h (limited hint)
    find_file(PDXKA_CURLVER_H curl/curlver.h HINTS ${CURL_INCLUDE_DIRS} NO_CACHE)
    if(PDXKA_CURLVER_H STREQUAL "PDXKA_CURLVER_H-NOTFOUND")
        # error if required, otherwise not found
        if(DEFINED _PDXKA_CURL_REQUIRED)
            message(FATAL_ERROR "Unable to find curl/curlver.h")
        else()
            set(CURL_FOUND FALSE)
        endif()
    endif()
    # ok, now find curl.exe (limited hint)
    find_program(
        PDXKA_CURL_EXE curl
        HINTS ${CURL_INSTALL_ROOT} PATH_SUFFIXES bin NO_CACHE
    )
    if(PDXKA_CURL_EXE STREQUAL "PDXKA_CURL_EXE-NOTFOUND")
        # error if required, otherwise not found
        if(DEFINED _PDXKA_CURL_REQUIRED)
            message(FATAL_ERROR "Unable to find curl executable")
        else()
            set(CURL_FOUND FALSE)
        endif()
    endif()
    # clear hint lists
    unset(_curl_root_hints)
    unset(_curl_include_hints)
    # find the libcurl library. for Windows the debug library is separate
    if(WIN32)
        find_file(
            CURL_LIBRARY libcurl_imp.lib
            HINTS ${CURL_INSTALL_ROOT} PATH_SUFFIXES lib NO_CACHE
        )
        find_file(
            CURL_DEBUG_LIBRARY libcurl-d_imp.lib
            HINTS ${CURL_INSTALL_ROOT} PATH_SUFFIXES lib NO_CACHE
        )
    else()
        find_file(
            CURL_LIBRARY libcurl.so
            HINTS ${CURL_INSTALL_ROOT} PATH_SUFFIXES lib NO_CACHE
        )
        set(CURL_DEBUG_LIBRARY ${CURL_LIBRARY})
    endif()
    if(CURL_LIBRARY STREQUAL "CURL_LIBRARY-NOTFOUND")
        # error if required, otherwise not found
        if(DEFINED _PDXKA_CURL_REQUIRED)
            message(FATAL_ERROR "Unable to find the libcurl library")
        else()
            set(CURL_FOUND FALSE)
        endif()
    endif()
    # only need to check debug library on Windows
    if(WIN32 AND CURL_DEBUG_LIBRARY STREQUAL "CURL_DEBUG_LIBRARY-NOTFOUND")
        if(DEFINED _PDXKA_CURL_REQUIRED)
            message(FATAL_ERROR "Unable to find the libcurl debug library")
        else()
            set(CURL_FOUND FALSE)
        endif()
    endif()
    # only run if CURL_FOUND is true
    if(CURL_FOUND)
        # to get version info, strip contents of curlver.h
        file(
            STRINGS "${PDXKA_CURLVER_H}"
            _curl_ver_info REGEX "#define[ \t]+LIBCURL_VERSION"
        )
        # get and strip version string (compatibility with FindCURL)
        list(GET _curl_ver_info 0 CURL_VERSION_STRING)
        string(STRIP "${CURL_VERSION_STRING}" CURL_VERSION_STRING)
        string(
            REGEX REPLACE "#define LIBCURL_VERSION[ \t]+" ""
            CURL_VERSION_STRING "${CURL_VERSION_STRING}"
        )
        string(REGEX REPLACE "\"" "" CURL_VERSION_STRING "${CURL_VERSION_STRING}")
        # more standard variable
        set(CURL_VERSION ${CURL_VERSION_STRING})
        # get and strip major version component
        list(GET _curl_ver_info 1 CURL_VERSION_MAJOR)
        string(STRIP "${CURL_VERSION_MAJOR}" CURL_VERSION_MAJOR)
        string(
            REGEX REPLACE "#define LIBCURL_VERSION_MAJOR[ \t]+" ""
            CURL_VERSION_MAJOR "${CURL_VERSION_MAJOR}"
        )
        # get and strip minor version component
        list(GET _curl_ver_info 2 CURL_VERSION_MINOR)
        string(STRIP "${CURL_VERSION_MINOR}" CURL_VERSION_MINOR)
        string(
            REGEX REPLACE "#define LIBCURL_VERSION_MINOR[ \t]+" ""
            CURL_VERSION_MINOR "${CURL_VERSION_MINOR}"
        )
        # get and strip patch version component
        list(GET _curl_ver_info 3 CURL_VERSION_PATCH)
        string(STRIP "${CURL_VERSION_PATCH}" CURL_VERSION_PATCH)
        string(
            REGEX REPLACE "#define LIBCURL_VERSION_PATCH[ \t]+" ""
            CURL_VERSION_PATCH "${CURL_VERSION_PATCH}"
        )
        # unset info list
        unset(_curl_ver_info)
        # run curl -V to get version info
        execute_process(
            COMMAND ${PDXKA_CURL_EXE} -V
            RESULT_VARIABLE PDXKA_CURL_VERSION_RESULT
            OUTPUT_VARIABLE PDXKA_CURL_VERSION_OUT
            ERROR_VARIABLE PDXKA_CURL_VERSION_ERR
        )
        # must succeed. nonzero is failure
        if(PDXKA_CURL_VERSION_RESULT)
            message(
                FATAL_ERROR
                "curl -V invocation failed.\nstdout:\n${PDXKA_CURL_VERSION_OUT}"
"stderr:\n${PDXKA_CURL_VERSION_ERR}"
            )
        endif()
        # convert output to list (should be 4 items)
        string(STRIP "${PDXKA_CURL_VERSION_OUT}" PDXKA_CURL_VERSION_OUT)
        string(REPLACE "\n" ";" PDXKA_CURL_VERSION_OUT "${PDXKA_CURL_VERSION_OUT}")
        # get protocols list (index 2)
        list(GET PDXKA_CURL_VERSION_OUT 2 PDXKA_CURL_PROTOCOLS)
        # remove prefix + convert into CMake list
        string(
            REPLACE "Protocols: " ""
            PDXKA_CURL_PROTOCOLS "${PDXKA_CURL_PROTOCOLS}"
        )
        string(REPLACE " " ";" PDXKA_CURL_PROTOCOLS "${PDXKA_CURL_PROTOCOLS}")
        # protocols should be uppercase to be compatible with FindCURL
        string(TOUPPER "${PDXKA_CURL_PROTOCOLS}" PDXKA_CURL_PROTOCOLS)
        # get features list (index 3)
        list(GET PDXKA_CURL_VERSION_OUT 3 PDXKA_CURL_FEATURES)
        string(
            REPLACE "Features: " ""
            PDXKA_CURL_FEATURES "${PDXKA_CURL_FEATURES}"
        )
        string(REPLACE " " ";" PDXKA_CURL_FEATURES "${PDXKA_CURL_FEATURES}")
        # perform version comparison if any
        if(DEFINED _PDXKA_CURL_VERSION)
            # uh oh, found version is not high enough
            if(_PDXKA_CURL_VERSION VERSION_GREATER CURL_VERSION)
                set(CURL_FOUND FALSE)
                # fatal error
                if(DEFINED _PDXKA_CURL_REQUIRED)
                    message(
                        FATAL_ERROR
                        "Found libcurl ${CURL_VERSION} < required version "
"${_PDXKA_CURL_VERSION}"
                    )
                # otherwise, information message
                else()
                    message(
                        STATUS
                        "Found libcurl ${CURL_VERSION} < requested version "
"${_PDXKA_CURL_VERSION}"
                    )
                endif()
            endif()
        endif()
        # information print of features and protocols. we first convert them to
        # be space-separated for nicer printing
        string(REPLACE ";" " " PDXKA_CURL_PRINT_FEATURES "${PDXKA_CURL_FEATURES}")
        string(REPLACE ";" " " PDXKA_CURL_PRINT_PROTOCOLS "${PDXKA_CURL_PROTOCOLS}")
        message(STATUS "libcurl features: ${PDXKA_CURL_PRINT_FEATURES}")
        message(STATUS "libcurl protocols: ${PDXKA_CURL_PRINT_PROTOCOLS}")
        # unset
        unset(PDXKA_CURL_PRINT_FEATURES)
        unset(PDXKA_CURL_PRINT_PROTOCOLS)
    endif()
    # CURL_FOUND may be false if version check failed. if not, then we can
    # begin matching the requested components
    if(CURL_FOUND AND DEFINED _PDXKA_CURL_COMPONENTS)
        # look for components
        foreach(_req_comp ${_PDXKA_CURL_COMPONENTS})
            # first check features
            foreach(_avail_feature ${PDXKA_CURL_FEATURES})
                if(_req_comp STREQUAL _avail_feature)
                    set(CURL_${_req_comp}_FOUND TRUE)
                    break()
                endif()
            endforeach()
            # check protocols if not found
            if(NOT DEFINED CURL_${_req_comp}_FOUND)
                foreach(_avail_proto ${PDXKA_CURL_PROTOCOLS})
                    if(_req_comp STREQUAL _avail_proto)
                        set(CURL_${_req_comp}_FOUND TRUE)
                        break()
                    endif()
                endforeach()
            endif()
            # if still not found, then we failed to locate the component
            if(NOT DEFINED CURL_${_req_comp}_FOUND)
                set(CURL_${_req_comp}_FOUND FALSE)
            endif()
        endforeach()
        # check that each component is found
        foreach(_req_comp ${_PDXKA_CURL_COMPONENTS})
            if(NOT CURL_${_req_comp}_FOUND)
                if(_PDXKA_CURL_REQUIRED)
                    message(
                        FATAL_ERROR
                        "Could NOT find requested libcurl component ${_req_comp}"
                    )
                else()
                    message(
                        STATUS
                        "Could NOT find requested libcurl component ${_req_comp}"
                    )
                    set(CURL_FOUND FALSE)
                endif()
            endif()
        endforeach()
    endif()
    # if CURL_FOUND still true after matching components, we define targets
    if(CURL_FOUND)
        add_library(CURL::libcurl INTERFACE IMPORTED)
        # note: on Win32 the library names are actually different
        target_link_libraries(
            CURL::libcurl INTERFACE
            $<IF:$<CONFIG:Debug>,${CURL_DEBUG_LIBRARY},${CURL_LIBRARY}>
        )
    endif()
    # finally, done
endmacro()
