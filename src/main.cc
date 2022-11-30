/**
 * @file main.cc
 * @author Derek Huang
 * @brief Main source file for the `xkcd-alt` tool.
 * @copyright MIT License
 */

#include <cstdlib>
#include <exception>
#include <iostream>
#include <string>

#include <boost/exception/diagnostic_information.hpp>

#include "pdxka/program_options.h"
#include "pdxka/rss.h"
#include "pdxka/string.h"

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
  const auto one_line = parse_result.map["one-line"].as<bool>();
  const auto previous = parse_result.map["back"].as<unsigned int>();
  const auto verbose = parse_result.map["verbose"].as<bool>();
  const auto insecure = parse_result.map["insecure"].as<bool>();
  // get XKCD RSS as a string using cURL
  const auto res = pdxka::get_rss(
    pdxka::curl_option{CURLOPT_VERBOSE, static_cast<long>(verbose)},
    pdxka::curl_option{CURLOPT_SSL_VERIFYPEER, static_cast<long>(!insecure)}
  );
  // if request error, just print the reason and exit
  PDXKA_CURL_NOT_OK(res.status) {
    std::cerr << "cURL error " << res.status << ": " << res.reason << std::endl;
    return EXIT_FAILURE;
  }
  // try to get XKCD RSS as a vector of rss_items, throw on error
  pdxka::rss_item_vector rss_items;
  try {
    rss_items = pdxka::to_item_vector(pdxka::parse_rss(res.payload));
  }
  catch (...) {
    std::cerr << boost::current_exception_diagnostic_information() << std::endl;
    return EXIT_FAILURE + 1;
  }
  // if empty, error out
  const auto n_items = rss_items.size();
  if (!n_items) {
    std::cerr << "Error: Couldn't find any one-liners in RSS feed!" << std::endl;
    return EXIT_FAILURE;
  }
  // if previous is size or greater, too far back
  if (previous >= n_items) {
    std::cerr << "Error: Can only go back at most " << n_items - 1 <<
      " strips, not " << previous << " strips" << std::endl;
    return EXIT_FAILURE;
  }
  // if printing as one line
  const auto& item = rss_items[previous];
  if (one_line)
    std::cout << item.img_title() << " -- " << item.guid();
  // else print fortune-style
  else
    std::cout << pdxka::line_wrap(item.img_title()) << "\n\t\t-- " << item.guid();
  // last newline + finally flush the buffer
  std::cout << std::endl;
  return EXIT_SUCCESS;
}
