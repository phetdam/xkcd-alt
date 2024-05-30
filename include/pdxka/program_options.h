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

#include "pdxka/features.h"
#include "pdxka/version.h"

#if PDXKA_USE_BOOST_PROGRAM_OPTIONS
#include <boost/program_options.hpp>
#else
#include <unordered_map>
#include <vector>
#endif  // !PDXKA_USE_BOOST_PROGRAM_OPTIONS

namespace pdxka {

#if PDXKA_USE_BOOST_PROGRAM_OPTIONS
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
 * @param argc Argument count from `main()`
 * @param argv Argument vector from `main()`
 */
option_parse_result parse_options(int argc, char* argv[]);
#else
/**
 * The command-line options map type.
 *
 * This is a simple map of string option name to a vector of argument values.
 */
using cliopt_map = std::unordered_map<std::string, std::vector<std::string>>;

/**
 * Parse command-line options for this application.
 *
 * @param map Command-line option argument map to populate
 * @param argc Argument count from `main()`
 * @param argv Argument vector from `main()`
 */
bool parse_options(cliopt_map& map, int argc, char* argv[]);
#endif  // !PDXKA_USE_BOOST_PROGRAM_OPTIONS

/**
 * Return the program's description text.
 */
inline const auto& program_description()
{
  static std::string desc{
    "Usage: " PDXKA_PROGNAME " [OPTION...]"
    "\n\n"
    "Prints the alt text for the most recent XKCD comic."
  };
  return desc;
}

/**
 * Return the program's version description text.
 */
inline const auto& version_description()
{
  static std::string desc{
    PDXKA_PROGNAME " " PDXKA_VERSION_STRING " (" PDXKA_BUILD_TYPE ", "
    PDXKA_SYSTEM_ARCH " " PDXKA_SYSTEM_NAME " " PDXKA_SYSTEM_VERSION ") "
    "libcurl/" PDXKA_LIBCURL_VERSION_STRING " "
    "libboost/" PDXKA_BOOST_VERSION_STRING " "
#if PDXKA_USE_BOOST_PROGRAM_OPTIONS
    "(headers program_options)"
#else
    "(headers)"
#endif  // !PDXKA_USE_BOOST_PROGRAM_OPTIONS
  };
  return desc;
}

}  // namespace pdxka

#endif  // PDXKA_PROGRAM_OPTIONS_H_
