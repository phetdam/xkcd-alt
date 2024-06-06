/**
 * @file program_main.hh
 * @author Derek Huang
 * @brief C++ header providing the main function of the pdxka CLI tool
 * @copyright MIT License
 */

#ifndef PDXKA_PROGRAM_MAIN_HH_
#define PDXKA_PROGRAM_MAIN_HH_

#include <cstdint>
#include <functional>
#include <limits>

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
 * Type alias for a callable that returns the XKCD RSS XML to parse.
 */
using rss_provider = std::function<curl_result(const cliopts&)>;

/**
 * `pdxka` CLI tool program main.
 *
 * This provides a hook for mocking in tests to avoid an actual network call.
 *
 * @param argc `argc` argument count from `main()`
 * @param argv `argv` argument vector from `main()`
 * @param provider Callable providing the RSS XML to parse
 * @returns `EXIT_SUCCESS` on success, `EXIT_FAILURE` or higher or failure
 */
int program_main(int argc, char* argv[], const rss_provider& provider);

/**
 * `pdxka` CLI tool program main convenience overload.
 *
 * This facilitates testing by removing an explicit `argc` specification.
 *
 * @tparam N Argument count deduced from `argv`
 *
 * @param argv `argv` argument vector from `main()`
 * @param provider Callable providing the RSS XML to parse
 * @returns `EXIT_SUCCESS` on success, `EXIT_FAILURE` or higher or failure
 */
template <std::size_t N>
inline auto program_main(char* (&argv)[N], const rss_provider& provider)
{
  // N must not exceed int max
  static_assert(N < std::numeric_limits<int>::max());
  return program_main(static_cast<int>(N), argv, provider);
}

}  // namespace pdxka

#endif  // PDXKA_PROGRAM_MAIN_HH_
