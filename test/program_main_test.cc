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
#include <tuple>
#include <utility>

#include <boost/test/data/test_case.hpp>
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
 * Mock a network call retrieving the XKCD RSS XML content.
 *
 * This returns the XKCD RSS page retrieved on 2024/06/04.
 *
 * @param opts Ignored command-line options struct
 */
pdxka::curl_result mock_rss_get(const pdxka::cliopts& /*opts*/)
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

}  // namespace

// XKCD alt text program tests
BOOST_AUTO_TEST_SUITE(xkcd_alt)

namespace {

/**
 * Callable object that returns the first `mock_program_main` input.
 *
 * This provides no arguments except the program name.
 */
struct argv_type_1 {
  auto operator()() const
  {
    // note: argument_vector CTAD fails with MSVC. see testing/program_main.hh
    // comments for why we have an extra function template for deduction
    return pt::make_argument_vector(PDXKA_PROGNAME);
  }
};

/**
 * Callable object that returns the second `mock_program_main` input.
 *
 * This requests an XKCD strip alt text 2 days previous from today.
 */
struct argv_type_2 {
  auto operator()() const
  {
    return pt::make_argument_vector(PDXKA_PROGNAME, "-b2");
  }
};

/**
 * Callable object that return the third `mock_program_main` input.
 *
 * This requests an XKCD strip alt text 3 days previous from today on one line.
 */
struct argv_type_3 {
  auto operator()() const
  {
    return pt::make_argument_vector(PDXKA_PROGNAME, "-o", "-b3");
  }
};

/**
 * Input type tuple for the `mock_program_main` test.
 */
using argv_type_tuple = std::tuple<argv_type_1, argv_type_2, argv_type_3>;

}  // namespace

/**
 * Run the program main using the RSS mocker on different CLI arguments.
 */
BOOST_AUTO_TEST_CASE_TEMPLATE(mock_program_main, T, argv_type_tuple)
{
  // run program main with RSS mocker, redirecting output to stringstream
  std::stringstream out;
  std::stringstream err_out;
  int ret;
  {
    pt::stream_diverter out_diverter{std::cout, out};
    pt::stream_diverter err_diverter{std::cerr, err_out};
    ret = pdxka::program_main(T{}(), mock_rss_get);
  }
  BOOST_TEST_REQUIRE(
    ret == EXIT_SUCCESS,
    "exit with nonzero status " << ret << ". error: " << err_out.str()
  );
}

BOOST_AUTO_TEST_SUITE_END()  // xkcd_alt
