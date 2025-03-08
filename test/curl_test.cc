/**
 * @file curl_test.cc
 * @author Derek Huang
 * @brief curl.hh unit tests
 * @copyright MIT License
 */

#include "pdxka/curl.hh"

#include <string>
#include <utility>

#include <boost/test/unit_test.hpp>
#include <curl/curl.h>

#include "pdxka/warnings.h"

BOOST_AUTO_TEST_SUITE(curl_test)

/**
 * Test that `PDXKA_CURL_ERR_HANDLER` works properly.
 *
 * @note Once the implementation of `PDXKA_CURL_ERR_HANDLER` was broken such
 *  that its use would be an infinite loop. Never again.
 */
BOOST_AUTO_TEST_CASE(curl_err_handler_test)
{
  // use too many redirects error to drive error handle
  std::string reason;
  char errbuf[] = "mock too many redirects";
// silence C3147 constant conditional
PDXKA_MSVC_WARNING_PUSH()
PDXKA_MSVC_WARNING_DISABLE(4127)
  PDXKA_CURL_ERR_HANDLER(CURLE_TOO_MANY_REDIRECTS, reason, errbuf, done);
PDXKA_MSVC_WARNING_POP()
  BOOST_TEST_FAIL("PDXKA_CURL_ERR_HANDLER failed to jump");
done:
  BOOST_TEST(true);
}

/**
 * Test that the `curl_handle` works properly for basic construction.
 *
 * Sets some easy options but does not perform anything.
 */
BOOST_AUTO_TEST_CASE(curl_handle_basic_test)
{
  pdxka::curl_handle handle;
  // set data to POST
  PDXKA_CURL_NOT_OK(curl_easy_setopt(handle, CURLOPT_POSTFIELDS, "some data"))
    BOOST_TEST_FAIL("failed to set CURLOPT_POSTFIELDS");
  // enable following redirects
  PDXKA_CURL_NOT_OK(curl_easy_setopt(handle, CURLOPT_FOLLOWLOCATION, 1L))
    BOOST_TEST_FAIL("failed to set CURLOPT_FOLLOWLOCATION");
}

/**
 * Test that the `curl_handle` works properly for move construction.
 */
BOOST_AUTO_TEST_CASE(curl_handle_move_ctor_test)
{
  // curl_handle wrapper and raw handle
  pdxka::curl_handle h1;
  auto h1h = h1.handle();
  // perform move construct
  auto h2 = std::move(h1);
  // h1 should be nullptr, h2 should acquire h1's handle
  BOOST_TEST_REQUIRE(!h1, "move from h1 failed");
  BOOST_TEST_REQUIRE(h2 == h1h, "move to h2 failed");
}

/**
 * Test that the `curl_handle` works properly for move assignment.
 */
BOOST_AUTO_TEST_CASE(curl_handle_move_asgn_test)
{
  // curl_handle objects and raw handle
  pdxka::curl_handle h1;
  pdxka::curl_handle h2;
  auto h1h = h1.handle();
  // perform move assignment
  h2 = std::move(h1);
  // h1 nullptr, h2 acquires h1's handle
  BOOST_TEST_REQUIRE(!h1, "move from h1 failed");
  BOOST_TEST_REQUIRE(h2 == h1h, "move to h2 failed");
}

BOOST_AUTO_TEST_SUITE_END()  // curl_test
