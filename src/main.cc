/**
 * @file main.cc
 * @author Derek Huang
 * @brief Main source file for the `xkcd-alt` tool.
 * @copyright MIT License
 */

#include <cstdlib>
#include <iostream>

#include <boost/program_options.hpp>

#include "pdxka/option_parser.h"

int main(int argc, char** argv)
{
  // parse command-line options + return exit code if error
  const auto parse_result = pdxka::parse_options(argc, argv);
  if (parse_result.exit_code) return parse_result.exit_code;
  // if help option was specified, print help output and exit
  if (parse_result.map.count("help")) {
    std::cout << parse_result.description << std::endl;
    return EXIT_SUCCESS;
  }
  // extract variables from parse_result variable map + get XKCD from RSS
  const auto attest = parse_result.map["attest"].as<bool>();
  const auto previous = parse_result.map["previous"].as<unsigned int>();
  // TODO: add actual implementation
  std::cout << "attest: " << attest << "\nprevious: " << previous << std::endl;
  return EXIT_SUCCESS;
}
