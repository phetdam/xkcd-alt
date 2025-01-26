/**
 * @file program_options.hh
 * @author Derek Huang
 * @brief C++ header for pdxka CLI option parsing
 * @copyright MIT License
 */

#ifndef PDXKA_PROGRAM_OPTIONS_HH_
#define PDXKA_PROGRAM_OPTIONS_HH_

#include <cstdlib>
#include <filesystem>
#include <string>

#include "pdxka/dllexport.h"
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
PDXKA_PUBLIC
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
PDXKA_PUBLIC
bool parse_options(cliopt_map& map, int argc, char* argv[]);
#endif  // !PDXKA_USE_BOOST_PROGRAM_OPTIONS

/**
 * Return the program's description text.
 *
 * When built with Boost.ProgramOptions this contains only a simple summary
 * description. However, when built using manual command-line option parsing,
 * this also contains all the help text for each command-line option.
 */
inline const auto& program_description()
{
  static std::string desc{
    "Usage: " PDXKA_PROGNAME
#if PDXKA_USE_BOOST_PROGRAM_OPTIONS
    " [OPTION...]\n"
#else
    " [-h] [-b[ ][BACK]] [-o] [-v] [-k]\n"
#endif  // !PDXKA_USE_BOOST_PROGRAM_OPTIONS
    "\n"
    "Prints the alt text for the most recent XKCD comic."
#if !PDXKA_USE_BOOST_PROGRAM_OPTIONS
    "\n"
    "\n"
    "Options:\n"
    "  -h, --help          Print this usage and exit\n"
    "  -V, --version       Print version information and exit\n"
    "\n"
    "  -b[ ][BACK], --back[=][BACK]\n"
    "                      Print alt text for the bth previous XKCD strip. If\n"
    "                      not given a value, implicitly sets b=1.\n"
    "\n"
    "  -o, --one-line      Print alt text and attestation on one line.\n"
    "  -v, --verbose       Allow cURL to print what's going on to stderr.\n"
    "                      Useful for debugging or satisfying curiosity.\n"
    "  -k, --insecure      Allow cURL to skip verification of the server's SSL\n"
    "                      certificate. Try not to specify this."
#endif  // !PDXKA_USE_BOOST_PROGRAM_OPTIONS
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

#endif  // PDXKA_PROGRAM_OPTIONS_HH_
