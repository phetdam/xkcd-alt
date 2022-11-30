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

#include <boost/program_options.hpp>

#include "pdxka/version.h"

namespace pdxka {

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

option_parse_result parse_options(int argc, char** argv);

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
