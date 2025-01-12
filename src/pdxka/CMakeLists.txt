cmake_minimum_required(VERSION ${CMAKE_MINIMUM_REQUIRED_VERSION})

# support library for PDXKA_PROGNAME executable target
add_library(pdxka program_options.cc program_main.cc rss.cc string.cc)
set_target_properties(pdxka PROPERTIES DEFINE_SYMBOL PDXKA_BUILD_DLL)
# when building as DLL definition of PDXKA_DLL is propagated downstream
if(BUILD_SHARED_LIBS)
    target_compile_definitions(pdxka PUBLIC PDXKA_DLL)
endif()
# need to link with PUBLIC since libcurl exposed through interface
# TODO: consider hiding this implementation detail? need to rework interface
# note: we don't actually have to link with libcurl here since the libcurl
# functions only get used in the header templates (used elsewhere)
target_link_libraries(pdxka PUBLIC CURL::libcurl)
# only link against Boost program_options if necessary
if(PDXKA_USE_BOOST_PROGRAM_OPTIONS)
    # also link public since Boost.ProgramOptions is exposed through interface
    # TODO: we could possibly just put the program_options.hh header in
    # pdxka/internal or just directly make it an internal header. currently none
    # of the other headers should be using it anyways
    target_link_libraries(pdxka PUBLIC Boost::program_options)
endif()
# on Windows don't forget to copy dependent DLLs if building shared
# note: if building static CMake will emit an error as TARGET_RUNTIME_DLLS
# doesn't make sense for a static library
if(WIN32 AND BUILD_SHARED_LIBS)
    pdxka_copy_runtime_dlls(pdxka)
endif()
