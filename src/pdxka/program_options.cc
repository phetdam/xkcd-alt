/**
 * @file program_options.cc
 * @author Derek Huang
 * @brief C++ source for pdxka CLI option parsing
 * @copyright MIT License
 */

#include "pdxka/program_options.hh"

#include <cstdlib>
#include <exception>
#include <filesystem>
#include <iostream>
#include <string>
#include <utility>

#include <boost/exception/diagnostic_information.hpp>

#include "pdxka/features.h"

#if PDXKA_USE_BOOST_PROGRAM_OPTIONS
#include <boost/program_options.hpp>
#else
#include <cstring>
#include <string_view>
#endif  // !PDXKA_USE_BOOST_PROGRAM_OPTIONS

namespace pdxka {

#if PDXKA_USE_BOOST_PROGRAM_OPTIONS
option_parse_result parse_options(int argc, char* argv[])
{
  // convenience namespace alias
  namespace po = boost::program_options;
  // general options group
  po::options_description desc(program_description() + "\n\nGeneral options");
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
    std::cerr << "Error: " << ex.what() << std::endl;
    exit_code = EXIT_FAILURE;
  }
  // any STL exceptions
  catch (const std::exception& ex) {
    std::cerr << "Error: STL exception: " << ex.what() << std::endl;
    exit_code = EXIT_FAILURE + 1;
  }
  // non-STL (ex. platform-specific) exceptions
  catch (...) {
    std::cerr << "Error: unknown exception: " <<
      boost::current_exception_diagnostic_information() << std::endl;
    exit_code = EXIT_FAILURE + 2;
  }
  // description and map objects should be moved to avoid copying
  return {exit_code, std::move(desc), std::move(vm)};
}
#else
bool parse_options(cliopt_map& opt_map, int argc, char* argv[])
{
  using mapped_type = typename std::decay_t<decltype(opt_map)>::mapped_type;
  // loop through arguments
  for (int i = 1; i < argc; i++) {
    // current argument (more convenient to use string_view)
    std::string_view arg{argv[i]};
    // print help
    if (arg == "-h" || arg == "--help")
      opt_map.try_emplace("help", mapped_type{});
    // print version
    else if (arg == "-V" || arg == "--version")
      opt_map.try_emplace("version", mapped_type{});
    // allow cURL to operate insecurely by skipping server SSL verification
    else if (arg == "-k" || arg == "--insecure")
      opt_map.try_emplace("insecure", mapped_type{});
    // run verbosely
    else if (arg == "-v" || arg == "--verbose")
      opt_map.try_emplace("verbose", mapped_type{});
    // print alt text and attestation on one line
    else if (arg == "-o" || arg == "--one-line")
      opt_map.try_emplace("one_line", mapped_type{});
    // option to print alt text for bth previous XKCD strip
    else if (arg == "-b" || arg == "--back") {
      // advance to find argument for number of strips, use 1 if none
      i++;
      // allows other options following -b, --back without arguments
      if (i >= argc || (std::strlen(argv[i]) && argv[i][0] == '-'))
        opt_map.insert_or_assign("back", mapped_type{"1"});
      // otherwise insert and allow overwriting
      else
        opt_map.insert_or_assign("back", mapped_type{argv[i]});
    }
    // short option to print alt text for bth previous XKCD strip, but the
    // number of strips to go back is appended to option, e.g. -b2
    else if (arg.substr(0, 2) == "-b")
      opt_map.insert_or_assign("back", mapped_type{arg.substr(2).data()});
    // long option to print alt text for bth previous XKCD strip, but the
    // number of strips to go back is appended, e.g. --back=2
    else if (arg.substr(0, 7) == "--back=")
      opt_map.insert_or_assign("back", mapped_type{arg.substr(7).data()});
    // unknown option
    else {
      std::cerr << "Error: unknown option " << arg << std::endl;
      return false;
    }
  }
  return true;
}
#endif  // !PDXKA_USE_BOOST_PROGRAM_OPTIONS

}  // namespace pdxka
