cmake_minimum_required(VERSION ${CMAKE_MINIMUM_REQUIRED_VERSION})

# support library for PDXKA_PROGNAME executable target
add_library(pdxka program_options.cc program_main.cc rss.cc string.cc)
# need to link with PUBLIC since cURL exposed through interface
# TODO: consider hiding this implementation detail? need to rework interface
target_link_libraries(pdxka PUBLIC CURL::libcurl)
# only link against Boost program_options if necessary
if(PDXKA_USE_BOOST_PROGRAM_OPTIONS)
    # also link public since Boost.ProgramOptions is exposed through interface
    # TODO: we could possibly just put the program_options.hh header in
    # pdxka/internal or just directly make it an internal header. currently none
    # of the other headers should be using it anyways
    target_link_libraries(pdxka PUBLIC Boost::program_options)
endif()
