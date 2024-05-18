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
#include <boost/timer/timer.hpp>

#include "pdxka/features.h"
#include "pdxka/testing/path.h"
#include "pdxka/version.h"

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
  if (err_output.size()) {
    std::cerr << "Error: Test produced output on stderr:\n\n" << err_output <<
      "\n" << std::endl;
    std::exit(EXIT_FAILURE);
  }
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
  if (bpos == std::string::npos) {
    std::cerr << "Error: Couldn't find \"libboost\" in following:\n\n" <<
      output << "\n" << std::endl;
    std::exit(EXIT_FAILURE);
  }
  // find the first '/'. if found, increment to get position of version
  auto bver_begin = output.find('/', bpos);
  if (bver_begin == std::string::npos) {
    std::cerr << "Error: Couldn't find appropriate Boost version in following:"
      "\n\n" << output << "\n" << std::endl;
    std::exit(EXIT_FAILURE);
  }
  bver_begin++;
  // find the next ' '. if found, this is one past the end of the version
  auto bver_end = output.find(' ', bver_begin);
  if (bver_end == std::string::npos) {
    std::cerr << "Error: Couldn't find end to Boost version in following:\n\n" <<
      output << "\n" << std::endl;
    std::exit(EXIT_FAILURE);
  }
  // Boost version string from output
  auto boost_version = output.substr(bver_begin, bver_end - bver_begin);
  if (boost_version != PDXKA_BOOST_VERSION_STRING) {
    std::cerr << "Error: Actual Boost version string " << boost_version <<
      " does not match expected version string " << PDXKA_BOOST_VERSION_STRING <<
      std::endl;
    std::exit(EXIT_FAILURE);
  }
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
  if (lparen_pos == std::string::npos) {
    std::cerr << "Error: Couldn't find left parenthesis marking Boost details" <<
      std::endl;
    std::exit(EXIT_FAILURE);
  }
  // find next ')'
  auto rparen_pos = output.find(')', lparen_pos);
  if (rparen_pos == std::string::npos) {
    std::cerr << "Error: No matching right parenthesis marking Boost details" <<
      std::endl;
    std::exit(EXIT_FAILURE);
  }
  // get entire parenthesized Boost details
  auto boost_details = output.substr(lparen_pos, rparen_pos - lparen_pos + 1);
  // check Boost details
#if PDXKA_USE_BOOST_PROGRAM_OPTIONS
  auto act_boost_details = "(headers program_options)";
#else
  auto act_boost_details = "(headers)";
#endif  // !PDXKA_USE_BOOST_PROGRAM_OPTIONS
  if (boost_details != act_boost_details) {
    std::cerr << "Error: Boost details " << boost_details << " do not match "
      "the expected " << act_boost_details << std::endl;
    std::exit(EXIT_FAILURE);
  }
}

}  // namespace

int main()
{
  namespace bp = boost::process;
  // timer for the current scope
  boost::timer::auto_cpu_timer scope_timer;
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
  return EXIT_SUCCESS;
}
