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
 * Given the program name, return the program's description.
 *
 * The program name will only display its base name, i.e. without directories.
 *
 * @param progname `const char*` `NULL`-terminated program name
 */
std::string program_description(const char* progname)
{
  std::filesystem::path progpath{progname};
  return
    "Usage: " + progpath.filename().string() + " [OPTIONS]"
    "\n\n"
    "Prints the alt text one-liner for today's XKCD comic."
    "\n\n"
    "Options";
}

/**
 * Parse command-line options for this application.
 *
 * Closely follows https://theboostcpplibraries.com/boost.program_options.
 *
 * @param argc `int` argument count
 * @param argv `char**` argument vector
 */
option_parse_results parse_options(int argc, char** argv)
{
  // convenience namespace alias
  namespace po = boost::program_options;
  // options description (with actual description)
  po::options_description desc(program_description(argv[0]));
  desc.add_options()
    ("help,h", "print this usage")
    ("attest,a", po::bool_switch(), "include XKCD strip title and URL")
    (
      "previous,b",
      po::value<unsigned int>()->default_value(0)->implicit_value(1),
      "print alt-text for bth previous XKCD strip. specifying the flag "
      "without a value implicitly sets b=1."
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
    std::exception_ptr exp = std::current_exception();
    std::cerr << "error: unknown exception: " <<
      boost::current_exception_diagnostic_information() << std::endl;
    exit_code = EXIT_FAILURE + 2;
  }
  return {exit_code, desc, vm};
}

}  // namespace pdxka
