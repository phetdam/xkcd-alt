/**
 * @file version_test.cc
 * @author Derek Huang
 * @brief C++ unit tests for version.h
 * @copyright MIT License
 */

#include "pdxka/version.h"

#include <string>

#include <boost/test/unit_test.hpp>
#include <curl/curlver.h>

#include "pdxka/testing/path.hh"
#include "pdxka/testing/process.hh"

// XKCD alt text program tests
BOOST_AUTO_TEST_SUITE(xkcd_alt)

/**
 * Test that the project's compile-time Boost version matches `BOOST_VERSION`.
 */
BOOST_AUTO_TEST_CASE(boost_compile_version)
{
  BOOST_TEST(PDXKA_BOOST_VERSION == BOOST_VERSION, "Boost version mismatch");
}

/**
 * Test that project's compile-time libcurl version matches the actual version.
 */
BOOST_AUTO_TEST_CASE(curl_compile_version)
{
  BOOST_TEST(
    PDXKA_LIBCURL_VERSION_STRING == LIBCURL_VERSION,
    "libcurl version mismatch"
  );
}

namespace {

/**
 * Check that the reported libcurl version is as expected.
 *
 * @param output Output written to `stdout`
 */
void check_curl_version(const std::string& output)
{
  // look for "libcurl" string
  auto cpos = output.find("libcurl");
  BOOST_TEST_REQUIRE(
    cpos != std::string::npos,
    "couldn't find \"libcurl\" in following:\n\n" << output << "\n"
  );
  // find next '/' that prefixes the version string
  auto cver_begin = output.find('/', cpos);
  BOOST_TEST_REQUIRE(
    cver_begin != std::string::npos,
    "couldn't find libcurl version in following:\n\n" << output << "\n"
  );
  // find end of libcurl version string by looking for next ' '
  auto cver_end = output.find(' ', ++cver_begin);
  BOOST_TEST_REQUIRE(
    cver_end != std::string::npos,
    "couldn't find end of libcurl version in following:\n\n" << output << "\n"
  );
  // get libcurl version substring and check against compile-time version
  auto curl_version = output.substr(cver_begin, cver_end - cver_begin);
  BOOST_TEST(
    PDXKA_LIBCURL_VERSION_STRING == curl_version,
    "runtime libcurl version " << curl_version <<
      " does not match compile-time version " << PDXKA_LIBCURL_VERSION_STRING
  );
}

}  // namespace

/**
 * Test that project's run-time libcurl version matches the actual version.
 */
BOOST_AUTO_TEST_CASE(curl_runtime_version)
{
  namespace pt = pdxka::testing;
  // invoke program with absolute path (so working directory is irrelevant)
  auto output = pt::run_process(pt::binary_dir() / PDXKA_PROGNAME, "-V");
  // require that process succeeded
  BOOST_TEST_REQUIRE(
    !output.error_code(),
    "process exited with non-zero status " << output.error_code().value()
  );
  // nothing should be written to stderr
  BOOST_TEST_REQUIRE(
    output.error_output().empty(),
    "test produced output on stderr: " << output.error_output()
  );
  // check that reported libcurl version is same PDXKA_LIBCURL_VERSION_STRING
  check_curl_version(output.output());
}

BOOST_AUTO_TEST_SUITE_END()  // xkcd_alt
