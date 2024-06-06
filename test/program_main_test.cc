/**
 * @file program_main_test.cc
 * @author Derek Huang
 * @brief C++ unit tests for program_main.hh
 * @copyright MIT License
 */

#include "pdxka/program_main.hh"

#include <cstdlib>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <sstream>
#include <string>
#include <utility>

#include <boost/test/unit_test.hpp>
#include <curl/curl.h>

#include "pdxka/curl.hh"
#include "pdxka/testing/path.hh"
#include "pdxka/testing/program_main.hh"
#include "pdxka/testing/stream_diverter.hh"
#include "pdxka/version.h"

namespace pt = pdxka::testing;

namespace {

/**
 * Callable mocking a network call retrieving the XKCD RSS XML content.
 */
struct rss_mocker {
  pdxka::curl_result operator()(const pdxka::cliopts& /*opts*/) const
  {
    // open stream to XML file content
    std::ifstream fs{(pt::data_dir() / "xkcd-rss-20240604.xml").string()};
    // read contents into string
    std::string rss_str;
    for (std::string line; std::getline(fs, line); )
      rss_str += line;
    // return new curl_result. move to avoid copy of rss_str
    return {CURLE_OK, "", pdxka::request_type::get, std::move(rss_str)};
  }
};

}  // namespace

// XKCD alt text program tests
BOOST_AUTO_TEST_SUITE(xkcd_alt)

/**
 * Run the program main using the RSS mocker with no CLI arguments.
 */
BOOST_AUTO_TEST_CASE(mock_program_main_default)
{
  pt::argument_vector argv{PDXKA_PROGNAME};
  // run program main with RSS mocker, redirecting output to stringstream
  std::stringstream out;
  std::stringstream err_out;
  int ret;
  {
    pt::stream_diverter out_diverter{std::cout, out};
    pt::stream_diverter err_diverter{std::cerr, err_out};
    ret = pdxka::program_main(argv, rss_mocker{});
  }
  BOOST_TEST_REQUIRE(ret == EXIT_SUCCESS, "exit failure. error: " << err_out.str());
}

/**
 * Run the program main using the RSS mocker going back 2 strips.
 */
BOOST_AUTO_TEST_CASE(mock_program_main_back2)
{
  pt::argument_vector argv{PDXKA_PROGNAME, "-b2"};
  // run program main with RSS mocker, redirecting output to stringstream
  std::stringstream out;
  std::stringstream err_out;
  int ret;
  {
    pt::stream_diverter out_diverter{std::cout, out};
    pt::stream_diverter err_diverter{std::cerr, err_out};
    ret = pdxka::program_main(argv, rss_mocker{});
  }
  BOOST_TEST_REQUIRE(ret == EXIT_SUCCESS, "exit failure. error: " << err_out.str());
}

BOOST_AUTO_TEST_SUITE_END()  // xkcd_alt
