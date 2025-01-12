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
# -h, --help tests
add_test(NAME ${_prog}_h COMMAND ${_prog} -h)
add_test(NAME ${_prog}_help COMMAND ${_prog} --help)
set_tests_properties(
    ${_prog}_h ${_prog}_help PROPERTIES
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

unset(_prog)
