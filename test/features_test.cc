/**
 * @file features_test.cc
 * @author Derek Huang
 * @brief C++ unit tests for features.h
 * @copyright MIT License
 */

#include "pdxka/features.h"

#include <cstdint>
#include <iostream>
#include <string>

#include <boost/filesystem/path.hpp>
#include <boost/process/child.hpp>
#include <boost/process/io.hpp>
#include <boost/process/pipe.hpp>
#include <boost/process/search_path.hpp>
#include <boost/test/unit_test.hpp>

#include "pdxka/features.h"
#include "pdxka/testing/path.hh"
#include "pdxka/testing/process.hh"
#include "pdxka/version.h"

namespace utf = boost::unit_test;

// XKCD alt text program tests
// note: if there are any fixtures being used across the whole suite it may be
// worth defining a new macro that uses a decorated BOOST_AUTO_TEST_SUITE
BOOST_AUTO_TEST_SUITE(xkcd_alt)

namespace {

/**
 * Check if there was any output to `stderr`.
 *
 * @param err_output Output written to `stderr`
 */
void check_err_output(const std::string& err_output)
{
  BOOST_TEST_REQUIRE(
    err_output.empty(),
    "test produced output on stderr:\n\n" << err_output << "\n"
  );
}

/**
 * Check that reported Boost version matches the compile-time string.
 *
 * @param output Output written to `stdout`
 * @returns Index of the character right after the end of the Boost version
 */
auto check_boost_version(const std::string& output)
{
  // search for Boost version. first find "libboost"
  auto bpos = output.find("libboost");
  BOOST_TEST_REQUIRE(
    bpos != std::string::npos,
    "couldn't find \"libboost\" in following:\n\n" << output << "\n"
  );
  // find the first '/'. if found, increment to get position of version
  auto bver_begin = output.find('/', bpos);
  BOOST_TEST_REQUIRE(
    bver_begin != std::string::npos,
    "couldn't find appropriate Boost version in following:\n\n" << output <<
      "\n"
  );
  // find the next ' '. if found, this is one past the end of the version
  auto bver_end = output.find(' ', ++bver_begin);
  BOOST_TEST_REQUIRE(
    bver_end != std::string::npos,
    "couldn't find end to Boost version in following:\n\n" << output << "\n"
  );
  // Boost version string from output
  auto boost_version = output.substr(bver_begin, bver_end - bver_begin);
  BOOST_TEST_REQUIRE(
    boost_version == PDXKA_BOOST_VERSION_STRING,
    "actual Boost version string " << boost_version <<
      " does not match expected version string " << PDXKA_BOOST_VERSION_STRING
  );
  // return end position
  return bver_end;
}

/**
 * Check that reported Boost component details is as expected.
 *
 * @param output Output written to `stdout`
 * @param pos Index of char right after the last char of the Boost version
 */
void check_boost_details(const std::string& output, std::size_t pos)
{
  // find next '('
  auto lparen_pos = output.find('(', pos);
  BOOST_TEST_REQUIRE(
    lparen_pos != std::string::npos,
    "couldn't find left parenthesis marking Boost details"
  );
  // find next ')'
  auto rparen_pos = output.find(')', lparen_pos);
  BOOST_TEST_REQUIRE(
    rparen_pos != std::string::npos,
    "no matching right parenthesis marking Boost details"
  );
  // get entire parenthesized Boost details
  auto boost_details = output.substr(lparen_pos, rparen_pos - lparen_pos + 1);
  // check Boost details
#if PDXKA_USE_BOOST_PROGRAM_OPTIONS
  auto act_boost_details = "(headers program_options)";
#else
  auto act_boost_details = "(headers)";
#endif  // !PDXKA_USE_BOOST_PROGRAM_OPTIONS
  BOOST_TEST_REQUIRE(
    boost_details == act_boost_details,
    "Boost details " << boost_details << " do not match the expected " <<
      act_boost_details
  );
}

}  // namespace

/**
 * Check that compile and run-time reporting of Boost program_options use match.
 *
 * We are just interested in checking that `PDXKA_USE_BOOST_PROGRAM_OPTIONS` is
 * consistent with the run-time reporting of the Boost components used.
 *
 * @note Consider removing since CTest already runs this test.
 */
BOOST_AUTO_TEST_CASE(bpo_check)
{
  namespace pt = pdxka::testing;
  // invoke program with absolute path (so working directory is irrelevant)
  auto output = pt::run_process(pt::program_path(), "-V");
  // require that process succeeded
  BOOST_TEST_REQUIRE(
    !output.error_code(),
    "process exited with non-zero status " << output.error_code().value()
  );
  // check for error output
  check_err_output(output.error_output());
  // check that runtime reported Boost version matches compile-time string +
  // check that the Boost components used are as expected
  check_boost_details(output.output(), check_boost_version(output.output()));
}

BOOST_AUTO_TEST_SUITE_END()  // xkcd_alt
