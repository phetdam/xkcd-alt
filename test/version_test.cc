/**
 * @file version_test.cc
 * @author Derek Huang
 * @brief C++ unit tests for version.h
 * @copyright MIT License
 */

#include "pdxka/version.h"

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

/**
 * Test that project's run-time libcurl version matches the actual version.
 */
BOOST_AUTO_TEST_CASE(curl_runtime_version, * boost::unit_test::disabled())
{
  namespace pt = pdxka::testing;
  // invoke program with absolute path (so working directory is irrelevant)
  auto output = pt::run_process(pt::binary_dir() / PDXKA_PROGNAME, "-V");
  // require that process succeeded
  BOOST_TEST_REQUIRE(
    !output.error_code(),
    "process exited with non-zero status " << output.error_code().value()
  );
  // TODO: add checks
}

BOOST_AUTO_TEST_SUITE_END()  // xkcd_alt
