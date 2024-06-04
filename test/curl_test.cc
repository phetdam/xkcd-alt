/**
 * @file curl_test.cc
 * @author Derek Huang
 * @brief curl.hh unit tests
 * @copyright MIT License
 */

#include "pdxka/curl.hh"

#include <string>

#include <boost/test/unit_test.hpp>
#include <curl/curl.h>

// libpdxka library tests
BOOST_AUTO_TEST_SUITE(libpdxka)

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
  PDXKA_CURL_ERR_HANDLER(CURLE_TOO_MANY_REDIRECTS, reason, errbuf, done);
  BOOST_TEST_FAIL("PDXKA_CURL_ERR_HANDLER failed to jump");
done:
  BOOST_TEST(true);
}

/**
 * Test that the `curl_handle` works properly.
 *
 * Sets some easy options but does not perform anything.
 */
BOOST_AUTO_TEST_CASE(curl_handle_test)
{
  pdxka::curl_handle handle;
  // set data to POST
  PDXKA_CURL_NOT_OK(curl_easy_setopt(handle, CURLOPT_POSTFIELDS, "some data"))
    BOOST_TEST_FAIL("failed to set CURLOPT_POSTFIELDS");
  // enable following redirects
  PDXKA_CURL_NOT_OK(curl_easy_setopt(handle, CURLOPT_FOLLOWLOCATION, 1L))
    BOOST_TEST_FAIL("failed to set CURLOPT_FOLLOWLOCATION");
}

BOOST_AUTO_TEST_SUITE_END()  // libpdxka
