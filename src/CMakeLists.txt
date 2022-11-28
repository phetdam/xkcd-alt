cmake_minimum_required(VERSION 3.21)

add_executable(${PDXKA_PROGNAME} main.cc option_parser.cc rss_parser.cc)
target_link_libraries(
    ${PDXKA_PROGNAME}
    ${PDXKA_HTTP_LIBRARY} Boost::program_options Threads::Threads
)
