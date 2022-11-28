/**
 * @file option_parser.h
 * @author Derek Huang
 * @brief Parse the CLI options using Boost.ProgramOptions
 * @copyright MIT License
 */

#include <string>

#include <boost/program_options.hpp>

namespace pdxka {

/**
 * Struct holding option parsing results.
 *
 * `exit_code` is the value `main` should return, `description` holds the
 * `options_description` object, and `map` holds the variable map.
 */
struct option_parse_results {
  const int exit_code;
  const boost::program_options::options_description description;
  const boost::program_options::variables_map map;
};

std::string program_description(const char* progname);
option_parse_results parse_options(int argc, char** argv);

}  // namespace pdxka
