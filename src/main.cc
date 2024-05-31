/**
 * @file main.cc
 * @author Derek Huang
 * @brief Main source file for the `xkcd-alt` tool.
 * @copyright MIT License
 */

#include "pdxka/program_main.hh"
#include "pdxka/rss.hh"

namespace {

/**
 * Callable that uses `pdxka::get_rss` to get the XKCD RSS content.
 *
 * @param opts Struct holding parsed command-line options
 * @returns `curl_result` with XKCD website RSS response
 */
auto get_xkcd_rss(const pdxka::cliopts& opts)
{
  return pdxka::get_rss(
    pdxka::curl_option<long>{CURLOPT_VERBOSE, opts.verbose},
    pdxka::curl_option<long>{CURLOPT_SSL_VERIFYPEER, !opts.insecure}
  );
}

}  // namespace

int main(int argc, char* argv[])
{
  return pdxka::program_main(argc, argv, get_xkcd_rss);
}
