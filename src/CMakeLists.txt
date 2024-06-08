cmake_minimum_required(VERSION ${CMAKE_MINIMUM_REQUIRED_VERSION})

# build pdxka library
add_subdirectory(pdxka)

add_executable(${PDXKA_PROGNAME} main.cc)
target_link_libraries(${PDXKA_PROGNAME} PRIVATE pdxka)
