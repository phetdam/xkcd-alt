/**
 * @file option_parser.cc
 * @author Derek Huang
 * @brief Parse the CLI options using Boost.ProgramOptions
 * @copyright MIT License
 */

#include "pdxka/option_parser.h"

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
  // options description (with actual description)
  po::options_description desc(program_description(argv[0]));
  desc.add_options()
    ("help,h", "Print this usage")
    (
      "verbose,v",
      po::bool_switch(),
      "Allow cURL to print what's going on to stderr. Useful for debugging "
      "or satisfying curiosity."
    )
    ("attest,a", po::bool_switch(), "Include XKCD strip title and URL")
    (
      "back,b",
      po::value<unsigned int>()->default_value(0)->implicit_value(1),
      "Print alt-text for bth previous XKCD strip. If not given a value, "
      "implicitly sets b=1."
    )
    (
      "insecure,k",
      po::bool_switch(),
      "Allow cURL to skip verification of the server's SSL certificate. Try "
      "not to specify this."
    )
  ;
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