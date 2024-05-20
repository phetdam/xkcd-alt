/**
 * @file pdxka_bpo_check.cc
 * @author Derek Huang
 * @brief C++ test checking whether built program uses Boost.ProgramOptions
 * @copyright MIT License
 */

#include <cstdint>
#include <cstdlib>
#include <iostream>
#include <string>

#include <boost/filesystem/path.hpp>
#include <boost/process/child.hpp>
#include <boost/process/io.hpp>
#include <boost/process/pipe.hpp>
#include <boost/process/search_path.hpp>

#include "pdxka/features.h"
#include "pdxka/testing/path.h"
#include "pdxka/version.h"

// TODO: consider permanent rewrite using Boost.Test
#if defined(PDXKA_BPO_CHECK_USE_BOOST_TEST)
/**
 * Name of Boost unit testing top-level test suite.
 */
#define BOOST_TEST_MODULE pdxka_bpo_check
#include <boost/test/unit_test.hpp>

// convenience macro to indicate if we are using Boost's test framework
#define USING_BOOST_TEST 1
#else
#define USING_BOOST_TEST 0
#include <boost/timer/timer.hpp>  // for boost::timer::auto_cpu_timer
#endif  // !defined(PDXKA_BPO_CHECK_USE_BOOST_TEST)

namespace {

/**
 * Check if there was any output to `stderr`.
 *
 * Prints message and exits with `EXIT_FAILURE` if `err_output` is non-empty.
 *
 * @param err_output Output written to `stderr`
 */
void check_err_output(const std::string& err_output)
{
#if USING_BOOST_TEST
  BOOST_TEST_REQUIRE(
    !err_output.size(),
    "test produced output on stderr:\n\n" << err_output << "\n"
  );
#else
  if (err_output.size()) {
    std::cerr << "Error: Test produced output on stderr:\n\n" << err_output <<
      "\n" << std::endl;
    std::exit(EXIT_FAILURE);
  }
#endif  // !USING_BOOST_TEST
}

/**
 * Check that reported Boost version matches the compile-time string.
 *
 * Prints message and exits with `EXIT_FAILURE` on errors.
 *
 * @param output Output written to `stdout`
 * @returns Index of the character right after the end of the Boost version
 */
auto check_boost_version(const std::string& output)
{
  // search for Boost version. first find "libboost"
  auto bpos = output.find("libboost");
#if USING_BOOST_TEST
  BOOST_TEST_REQUIRE(
    bpos != std::string::npos,
    "couldn't find \"libboost\" in following:\n\n" << output << "\n"
  );
#else
  if (bpos == std::string::npos) {
    std::cerr << "Error: Couldn't find \"libboost\" in following:\n\n" <<
      output << "\n" << std::endl;
    std::exit(EXIT_FAILURE);
  }
#endif  // !USING_BOOST_TEST
  // find the first '/'. if found, increment to get position of version
  auto bver_begin = output.find('/', bpos);
#if USING_BOOST_TEST
  BOOST_TEST_REQUIRE(
    bver_begin != std::string::npos,
    "couldn't find appropriate Boost version in following:\n\n" << output <<
      "\n"
  );
#else
  if (bver_begin == std::string::npos) {
    std::cerr << "Error: Couldn't find appropriate Boost version in following:"
      "\n\n" << output << "\n" << std::endl;
    std::exit(EXIT_FAILURE);
  }
#endif  // !USING_BOOST_TEST
  // find the next ' '. if found, this is one past the end of the version
  auto bver_end = output.find(' ', ++bver_begin);
#if USING_BOOST_TEST
  BOOST_TEST_REQUIRE(
    bver_end != std::string::npos,
    "couldn't find end to Boost version in following:\n\n" << output << "\n"
  );
#else
  if (bver_end == std::string::npos) {
    std::cerr << "Error: Couldn't find end to Boost version in following:\n\n" <<
      output << "\n" << std::endl;
    std::exit(EXIT_FAILURE);
  }
#endif  // !USING_BOOST_TEST
  // Boost version string from output
  auto boost_version = output.substr(bver_begin, bver_end - bver_begin);
#if USING_BOOST_TEST
  BOOST_TEST_REQUIRE(
    boost_version == PDXKA_BOOST_VERSION_STRING,
    "actual Boost version string " << boost_version <<
      " does not match expected version string " << PDXKA_BOOST_VERSION_STRING
  );
#else
  if (boost_version != PDXKA_BOOST_VERSION_STRING) {
    std::cerr << "Error: Actual Boost version string " << boost_version <<
      " does not match expected version string " << PDXKA_BOOST_VERSION_STRING <<
      std::endl;
    std::exit(EXIT_FAILURE);
  }
#endif // !USING_BOOST_TEST
  // return end position
  return bver_end;
}

/**
 * Check that reported Boost component details is as expected.
 *
 * Prints message and exits with `EXIT_FAILURE` on errors.
 *
 * @param output Output written to `stdout`
 * @param pos Index of char right after the last char of the Boost version
 */
void check_boost_details(const std::string& output, std::size_t pos)
{
  // find next '('
  auto lparen_pos = output.find('(', pos);
#if USING_BOOST_TEST
  BOOST_TEST_REQUIRE(
    lparen_pos != std::string::npos,
    "couldn't find left parenthesis marking Boost details"
  );
#else
  if (lparen_pos == std::string::npos) {
    std::cerr << "Error: Couldn't find left parenthesis marking Boost details" <<
      std::endl;
    std::exit(EXIT_FAILURE);
  }
#endif // !USING_BOOST_TEST
  // find next ')'
  auto rparen_pos = output.find(')', lparen_pos);
#if USING_BOOST_TEST
  BOOST_TEST_REQUIRE(
    rparen_pos != std::string::npos,
    "no matching right parenthesis marking Boost details"
  );
#else
  if (rparen_pos == std::string::npos) {
    std::cerr << "Error: No matching right parenthesis marking Boost details" <<
      std::endl;
    std::exit(EXIT_FAILURE);
  }
#endif  // !USING_BOOST_TEST
  // get entire parenthesized Boost details
  auto boost_details = output.substr(lparen_pos, rparen_pos - lparen_pos + 1);
  // check Boost details
#if PDXKA_USE_BOOST_PROGRAM_OPTIONS
  auto act_boost_details = "(headers program_options)";
#else
  auto act_boost_details = "(headers)";
#endif  // !PDXKA_USE_BOOST_PROGRAM_OPTIONS
#if USING_BOOST_TEST
  BOOST_TEST_REQUIRE(
    boost_details == act_boost_details,
    "Boost details " << boost_details << " do not match the expected " <<
      act_boost_details
  );
#else
  if (boost_details != act_boost_details) {
    std::cerr << "Error: Boost details " << boost_details << " do not match "
      "the expected " << act_boost_details << std::endl;
    std::exit(EXIT_FAILURE);
  }
#endif // !USING_BOOST_TEST
}

}  // namespace

#if USING_BOOST_TEST
// XKCD alt text program tests
BOOST_AUTO_TEST_SUITE(xkcd_alt)

BOOST_AUTO_TEST_CASE(bpo_check)
#else
int main()
#endif  // !USING_BOOST_TEST
{
  namespace bp = boost::process;
#if !USING_BOOST_TEST
  // timer for the current scope. for tangible output when not using Boost.Test
  boost::timer::auto_cpu_timer scope_timer;
#endif  // !USING_BOOST_TEST
  // streams for redirected stdout and stderr
  bp::ipstream out_stream;
  bp::ipstream err_stream;
  // invoke child process (absolute path so working directory is irrelevant)
  bp::child child{
    boost::filesystem::path{PDXKA_BINARY_DIR} / PDXKA_PROGNAME, "-V",
    bp::std_out > out_stream,
    bp::std_err > err_stream
  };
  // current stdout + stderr lines and full stdout + stderr output
  std::string out_line;
  std::string err_line;
  std::string output;
  std::string err_output;
  // synchronously collect output
  while (child.running()) {
    if (std::getline(out_stream, out_line) && out_line.size())
      output += out_line;
    if (std::getline(err_stream, err_line) && err_line.size())
      err_output += err_line;
  }
  // check for error output
  check_err_output(err_output);
  // check that runtime reported Boost version matches compile-time string +
  // check that the Boost components used are as expected
  check_boost_details(output, check_boost_version(output));
#if !USING_BOOST_TEST
  return EXIT_SUCCESS;
#endif  // !USING_BOOST_TEST
}

#if USING_BOOST_TEST
BOOST_AUTO_TEST_SUITE_END()  // xkcd_alt
#endif  // USING_BOOST_TEST
