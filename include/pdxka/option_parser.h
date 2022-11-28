/**
 * @file option_parser.h
 * @author Derek Huang
 * @brief Parse the CLI options using Boost.ProgramOptions
 * @copyright MIT License
 */

#ifndef PDXKA_OPTION_PARSER_H_
#define PDXKA_OPTION_PARSER_H_

#include <cstdlib>
#include <filesystem>
#include <string>

#include <boost/program_options.hpp>

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
    "Usage: " + progpath.filename().string() + " [OPTIONS]"
    "\n\n"
    "Prints the alt text one-liner for today's XKCD comic."
    "\n\n"
    "Options";
}

option_parse_result parse_options(int argc, char** argv);

}  // namespace pdxka

#endif  // PDXKA_OPTION_PARSER_H_
