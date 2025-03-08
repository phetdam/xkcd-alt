/**
 * @file posix_test.cc
 * @author Derek Huang
 * @brief posix.hh unit tests
 * @copyright MIT License
 */

#include "pdxka/posix.hh"

#include <boost/test/unit_test.hpp>

BOOST_AUTO_TEST_SUITE(posix_test)

/**
 * Test that default-constructed `posix_error` using `errno` works as expected.
 */
BOOST_AUTO_TEST_CASE(from_errno_test)
{
  using pdxka::posix_error;
  // expected + actual errno value
  constexpr int exp_err = EINVAL;
  int act_err = 0;
  // set errno to prepare test
  errno = exp_err;
  // catch throw
  try {
    throw posix_error{};
  }
  catch (const posix_error& exc) {
    act_err = exc.err();
  }
  // must be equal and act_err must be nonzero
  BOOST_TEST_REQUIRE(act_err);
  BOOST_TEST(exp_err == act_err);
}

BOOST_AUTO_TEST_SUITE_END()  // posix_test
