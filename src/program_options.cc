/**
 * @file program_options.cc
 * @author Derek Huang
 * @brief Parse the CLI options using Boost.ProgramOptions
 * @copyright MIT License
 */

#include "pdxka/program_options.h"

#include <cstdlib>
#include <exception>
#include <filesystem>
#include <iostream>
#include <string>

#include <boost/exception/diagnostic_information.hpp>
#include <boost/program_options.hpp>

namespace pdxka {

/**
 * Parse command-line options for this application.
 *
 * Closely follows https://theboostcpplibraries.com/boost.program_options.
 *
 * @param argc `int` argument count
 * @param argv `char**` argument vector
 */
option_parse_result parse_options(int argc, char** argv)
{
  // convenience namespace alias
  namespace po = boost::program_options;
  // general options group
  po::options_description desc(program_description(argv[0]) + "\n\nGeneral options");
  desc.add_options()
    (
      "back,b",
      po::value<unsigned int>()->default_value(0)->implicit_value(1),
      "Print alt-text for bth previous XKCD strip. If not given a value, "
      "implicitly sets b=1."
    )
    (
      "one-line,o",
      po::bool_switch(),
      "Print alt text and attestation on one line."
    )
  ;
  // debug options group
  po::options_description desc_debug("Debug options");
  desc_debug.add_options()
    (
      "verbose,v",
      po::bool_switch(),
      "Allow cURL to print what's going on to stderr. Useful for debugging "
      "or satisfying curiosity."
    )
    (
      "insecure,k",
      po::bool_switch(),
      "Allow cURL to skip verification of the server's SSL certificate. Try "
      "not to specify this."
    )
  ;
  // "other" options group
  po::options_description desc_other("Other options");
  desc_other.add_options()
    ("help,h", "Print this usage and exit")
    ("version,V", "Print version information and exit")
  ;
  // add debug + other options to top-level options description
  desc.add(desc_debug).add(desc_other);
  // variable map storing options + exit code main should return
  po::variables_map vm;
  int exit_code = EXIT_SUCCESS;
  // try to parse and store options, catch errors as necessary
  try {
    po::store(po::parse_command_line(argc, argv, desc), vm);
    po::notify(vm);
  }
  // option parsing errors
  catch (const po::error& ex) {
    std::cerr << "error: " << ex.what() << std::endl;
    exit_code = EXIT_FAILURE;
  }
  // any STL exceptions
  catch (const std::exception& ex) {
    std::cerr << "error: STL exception: " << ex.what() << std::endl;
    exit_code = EXIT_FAILURE + 1;
  }
  // non-STL (ex. platform-specific) exceptions
  catch (...) {
    std::cerr << "error: unknown exception: " <<
      boost::current_exception_diagnostic_information() << std::endl;
    exit_code = EXIT_FAILURE + 2;
  }
  return {exit_code, desc, vm};
}

}  // namespace pdxka
