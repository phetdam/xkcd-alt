cmake_minimum_required(VERSION ${CMAKE_MINIMUM_REQUIRED_VERSION})

# support library for PDXKA_PROGNAME executable target
add_library(pdxka program_options.cc program_main.cc rss.cc string.cc)
# need to link with PUBLIC since cURL exposed through interface
# TODO: consider hiding this implementation detail? need to rework interface
target_link_libraries(pdxka PUBLIC CURL::libcurl)
