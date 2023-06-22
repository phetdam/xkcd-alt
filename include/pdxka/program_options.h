/**
 * @file program_options.h
 * @author Derek Huang
 * @brief Parse the CLI options using Boost.ProgramOptions
 * @copyright MIT License
 */

#ifndef PDXKA_PROGRAM_OPTIONS_H_
#define PDXKA_PROGRAM_OPTIONS_H_

#include <cstdlib>
#include <filesystem>
#include <string>

#include "pdxka/version.h"

#ifdef PDXKA_USE_BOOST_PROGRAM_OPTIONS
#include <boost/program_options.hpp>
// for now, we don't have any alternative implementation
#else
#error "pdxka/program_options.h: Must define PDXKA_USE_BOOST_PROGRAM_OPTIONS"
#endif  // PDXKA_USE_BOOST_PROGRAM_OPTIONS

namespace pdxka {

#ifdef PDXKA_USE_BOOST_PROGRAM_OPTIONS
/**
 * Struct holding option parsing results.
 *
 * `exit_code` is the value `main` should return, `description` holds the
 * `options_description` object, and `map` holds the variable map.
 */
struct option_parse_result {
  const int exit_code;
  const boost::program_options::options_description description;
  const boost::program_options::variables_map map;
};

/**
 * Parse command-line options for this application.
 *
 * Closely follows https://theboostcpplibraries.com/boost.program_options.
 *
 * @param argc `int` argument count
 * @param argv `char**` argument vector
 */
option_parse_result parse_options(int argc, char** argv);
#endif  // PDXKA_USE_BOOST_PROGRAM_OPTIONS

/**
 * Given the program name, return the program's description.
 *
 * The program name will only display its base name, i.e. without directories.
 *
 * @param progname `const char*` `NULL`-terminated program name
 */
inline std::string program_description(const char* progname)
{
  std::filesystem::path progpath{progname};
  return
    "Usage: " + progpath.filename().string() + " [OPTION...]"
    "\n\n"
    "Prints the alt text for the most recent XKCD comic.";
}

/**
 * Given the program name, return the program's version description.
 *
 * @param progname `const char*` `NULL`-terminated program name
 */
inline std::string version_description(const char* progname)
{
  std::filesystem::path progpath{progname};
  return
    progpath.filename().string() + " " + pdxka::version + " (" +
    pdxka::build_type + ", " + pdxka::system_arch + " " +
    pdxka::system_name + " " + pdxka::system_version + ")";
}

}  // namespace pdxka

#endif  // PDXKA_PROGRAM_OPTIONS_H_
