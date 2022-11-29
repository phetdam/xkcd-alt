/**
 * @file main.cc
 * @author Derek Huang
 * @brief Main source file for the `xkcd-alt` tool.
 * @copyright MIT License
 */

#include <cstdlib>
#include <exception>
#include <iostream>

#include "pdxka/option_parser.h"
#include "pdxka/rss_parser.h"

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
  // extract variables from parse_result variable map
  const auto attest = parse_result.map["attest"].as<bool>();
  const auto previous = parse_result.map["back"].as<unsigned int>();
  // get XKCD RSS as a vector of rss_items
  const auto res = pdxka::get_rss();  // TODO: allow more options
  const auto rss_items = pdxka::to_item_vector(pdxka::parse_rss(res.payload));
  // dummy: print all the titles and subtitles
  for (const auto& rss_item : rss_items)
    std::cout << rss_item.title() << " -- " << rss_item.img_title() << "\n";
  // dummy: print int values of attest and previous
  std::cout << "attest=" << attest << ", previous=" << previous << std::endl;
  return EXIT_SUCCESS;
}
