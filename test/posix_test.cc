/**
 * @file posix_test.cc
 * @author Derek Huang
 * @brief posix.hh unit tests
 * @copyright MIT License
 */

#include "pdxka/posix.hh"

#include <string>
#include <thread>
#include <vector>

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

/**
 * Test that `pdxka::strerror` works in a multithreaded context.
 */
BOOST_AUTO_TEST_CASE(strerror_mt_test)
{
  // number of thrads/messages
  constexpr auto n_threads = 128u;
  // vector to hold error strings. we cannot use const char* because the buffer
  // pointer returned is not guaranteed to be valid when a thread exits
  std::vector<std::string> errstrs(n_threads);
  // launch threads
  std::vector<std::thread> threads;
  for (unsigned i = 0; i < n_threads; i++)
    threads.emplace_back([i, &errstrs] { errstrs[i] = pdxka::strerror(ENOMEM); });
  // join and compare
  for (auto& thread : threads)
    thread.join();
  for (unsigned i = 0; i < n_threads - 1; i++)
    BOOST_TEST(
      errstrs[i] == errstrs[i + 1],
      "errstrs[" << i << "] != errstrs[" << i << " + 1] [\"" <<
        errstrs[i] << "\" != \"" << errstrs[i + 1] << "\"]"
    );
}

BOOST_AUTO_TEST_SUITE_END()  // posix_test
