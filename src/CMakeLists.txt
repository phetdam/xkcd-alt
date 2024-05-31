cmake_minimum_required(VERSION ${CMAKE_MINIMUM_REQUIRED_VERSION})

# build pdxka library
add_subdirectory(pdxka)

add_executable(${PDXKA_PROGNAME} main.cc)
target_link_libraries(${PDXKA_PROGNAME} PRIVATE pdxka)
# only link against Boost program_options if necessary
if(PDXKA_USE_BOOST_PROGRAM_OPTIONS)
    target_link_libraries(${PDXKA_PROGNAME} PRIVATE Boost::program_options)
endif()
