cmake_minimum_required(VERSION ${CMAKE_MINIMUM_REQUIRED_VERSION})

add_executable(${PDXKA_PROGNAME} main.cc program_options.cc rss.cc string.cc)
# need to link with PUBLIC since cURL exposed through interface
target_link_libraries(${PDXKA_PROGNAME} PUBLIC CURL::libcurl)
# only link against Boost program_options if necessary
if(PDXKA_USE_BOOST_PROGRAM_OPTIONS)
    target_link_libraries(${PDXKA_PROGNAME} PUBLIC Boost::program_options)
endif()
