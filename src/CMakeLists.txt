cmake_minimum_required(VERSION ${CMAKE_MINIMUM_REQUIRED_VERSION})

# build pdxka library
add_subdirectory(pdxka)

# helper variable to shorten usage of PDXKA_PROGNAME
set(_prog ${PDXKA_PROGNAME})

# xkcd-alt: program to print XKCD alt text
add_executable(${_prog} main.cc)
target_link_libraries(${_prog} PRIVATE pdxka)
# on Windows don't forget to copy dependent DLLs
if(WIN32)
    pdxka_copy_runtime_dlls(${_prog})
endif()

# install rule + target export rule
install(TARGETS ${_prog} EXPORT pdxka_exports)
install(
    EXPORT pdxka_exports
    DESTINATION ${PDXKA_CMAKE_PREFIX}
    NAMESPACE ${PROJECT_NAME}::
    FILE ${PROJECT_NAME}-targets.cmake
)

# -h, --help tests
add_test(NAME ${_prog}_h COMMAND ${_prog} -h)
add_test(NAME ${_prog}_help COMMAND ${_prog} --help)
# test that -b can be specified with -h, --help. this in general is just to
# test that manual parsing of CLI options don't skip the next arg after -b if
# it is another CLI option that start with a hyphen.
add_test(NAME ${_prog}_b_h COMMAND ${_prog} -b -h)
add_test(NAME ${_prog}_b_help COMMAND ${_prog} -b --help)
set_tests_properties(
    ${_prog}_h ${_prog}_help ${_prog}_b_h ${_prog}_b_help PROPERTIES
    PASS_REGULAR_EXPRESSION "Prints the alt text for the most recent XKCD comic"
)
# -V, --version tests
add_test(NAME ${_prog}_V COMMAND ${_prog} -V)  # note capital "V"
add_test(NAME ${_prog}_version COMMAND ${_prog} --version)
set_tests_properties(
    ${_prog}_V ${_prog}_version PROPERTIES
    PASS_REGULAR_EXPRESSION
    "libcurl/${CURL_VERSION_STRING} libboost/${Boost_VERSION_STRING}"
)
# if -h, --help or -V, --version tests actually get RSS, error
set_tests_properties(
    ${_prog}_h
    ${_prog}_help
    ${_prog}_b_h
    ${_prog}_b_help
    ${_prog}_V
    ${_prog}_version PROPERTIES
    FAIL_REGULAR_EXPRESSION "-- https://xkcd.com"
)

unset(_prog)
