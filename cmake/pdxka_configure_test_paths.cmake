cmake_minimum_required(VERSION 3.13)

##
# pdxka_configure_test_paths.cmake
#
# Run configure_file on testing/path.hh.in as a pre-build step so separate
# path.hh files can be generated per build config for multi-config generators.
# This ensures that the per-config binary directory listed in the header is
# accurate in the case of multi-config generators.
#
# External variables consumed:
#
#   PDXKA_TEST_PATHS_HEADER_IN      Path to the path.hh.in input in source tree
#   PDXKA_TEST_PATHS_HEADER         Path to the path.hh output to generate
#   PDXKA_BINARY_DIR
#   PDXKA_DATA_DIR
#

# execute only in CMake script mode
if(CMAKE_SCRIPT_MODE_FILE)
    # configure using consumed variables
    configure_file(
        ${PDXKA_TEST_PATHS_HEADER_IN} ${PDXKA_TEST_PATHS_HEADER}
        @ONLY NEWLINE_STYLE LF
    )
endif()
