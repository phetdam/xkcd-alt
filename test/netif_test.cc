/**
 * @file netif_test.cc
 * @author Derek Huang
 * @brief netif.hh unit tests
 * @copyright MIT License
 */

#include "pdxka/netif.hh"

#include <net/if.h>

#include <cstdint>
#include <iterator>
#include <type_traits>
#include <utility>

#include <boost/test/unit_test.hpp>

BOOST_AUTO_TEST_SUITE(netif_test)

/**
 * Test `netifaddrs_list` standard construction.
 */
BOOST_AUTO_TEST_CASE(netifaddrs_ctor_test)
{
  pdxka::netifaddrs_list netifs;
}

/**
 * Test `netifaddrs_list` disabled copy construction.
 */
BOOST_AUTO_TEST_CASE(netifaddrs_copy_ctor_test)
{
  BOOST_TEST(!std::is_copy_constructible_v<pdxka::netifaddrs_list>);
}

/**
 * Test `netifaddrs_list` disabled copy assignment.
 */
BOOST_AUTO_TEST_CASE(netifaddrs_copy_assign_test)
{
  BOOST_TEST(!std::is_copy_assignable_v<pdxka::netifaddrs_list>);
}

/**
 * Test that `netifaddrs_list` move construction works.
 */
BOOST_AUTO_TEST_CASE(netifaddrs_move_ctor_test)
{
  pdxka::netifaddrs_list nfs1;
  auto nfs2 = std::move(nfs1);
  // nfs1 should be empty and nfs2 should at least have the loopback device
  BOOST_TEST(!nfs1);
  BOOST_TEST(nfs2);
}

/**
 * Test that `netifaddrs_list` move assignment works.
 */
BOOST_AUTO_TEST_CASE(netifaddrs_move_assign_test)
{
  pdxka::netifaddrs_list nfs1, nfs2;
  nfs2 = std::move(nfs1);
  // nfs1 should be empty and nfs2 should at least have the loopback device
  BOOST_TEST(!nfs1);
  BOOST_TEST(nfs2);
}

/**
 * Test `netifaddrs_list` compatibility with `std::distance`.
 */
BOOST_AUTO_TEST_CASE(netifaddrs_distance_test)
{
  pdxka::netifaddrs_list netifs;
  BOOST_TEST(std::distance(std::begin(netifs), std::end(netifs)));
}

BOOST_AUTO_TEST_SUITE_END()  // netif_test
