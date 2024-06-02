/**
 * @file program_main.hh
 * @author Derek Huang
 * @brief C++ header providing the main function of the pdxka CLI tool
 * @copyright MIT License
 */

#ifndef PDXKA_PROGRAM_MAIN_HH_
#define PDXKA_PROGRAM_MAIN_HH_

#include <functional>

#include "pdxka/curl.hh"

namespace pdxka {

/**
 * Struct holding parsed command-line options.
 *
 * @param one_line Flag to indicate if output should be printed on one line
 * @param previous Number of XKCD strips to go back from today's strip
 * @param verbose Flag to operate cURL in verbose mode
 * @param insecure Flag to allow skip cURL verification of server SSL cert
 */
struct cliopts {
  const bool one_line;
  const unsigned int previous;
  const bool verbose;
  const bool insecure;
};

/**
 * `pdxka` CLI tool program main.
 *
 * This provides a hook for mocking in tests to avoid an actual network call.
 *
 * @todo Consider making this `extern` and C++ified for later testing.
 *
 * @param argc `argc` argument count from `main()`
 * @param argv `argv` argument vector from `main()
 * @param rss_factory Callable providing the `curl_result` to parse
 * @returns `EXIT_SUCCESS` on success, `EXIT_FAILURE` or higher or failure
 */
int program_main(
  int argc,
  char* argv[],
  const std::function<curl_result(const cliopts&)>& rss_factory);

}  // namespace pdxka

#endif  // PDXKA_PROGRAM_MAIN_HH_
