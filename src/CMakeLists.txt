cmake_minimum_required(VERSION 3.21)

add_executable(${PDXKA_PROGNAME} main.cc program_options.cc rss.cc string.cc)
target_link_libraries(
    ${PDXKA_PROGNAME}
    ${PDXKA_HTTP_LIBRARY} Boost::program_options CURL::libcurl
)
