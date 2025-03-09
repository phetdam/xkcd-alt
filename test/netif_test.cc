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
#include <tuple>
#include <type_traits>
#include <utility>

#include <boost/test/unit_test.hpp>

#include "pdxka/type_traits.hh"

BOOST_AUTO_TEST_SUITE(netif_test)

namespace {

/**
 * Test fixture with a single `netifaddrs_list`.
 */
struct netifaddrs_fixture_1 {
  pdxka::netifaddrs_list nifs_;  // tests default construction
};

/**
 * Test fixture with two `netifaddrs_list` instances.
 */
struct netifaddrs_fixture_2 {
  pdxka::netifaddrs_list nifs1_;
  pdxka::netifaddrs_list nifs2_;
};

}  // namespace

/**
 * Test `netifaddrs_list` standard construction.
 */
BOOST_FIXTURE_TEST_CASE(netifaddrs_ctor_test, netifaddrs_fixture_1)
{
  // for picky compilers
  (void) nifs_;
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
BOOST_FIXTURE_TEST_CASE(netifaddrs_move_ctor_test, netifaddrs_fixture_1)
{
  auto nifs2 = std::move(nifs_);
  // nifs_ should be empty and nifs2 should at least have the loopback device
  BOOST_TEST(!nifs_);
  BOOST_TEST(nifs2);
}

/**
 * Test that `netifaddrs_list` move assignment works.
 */
BOOST_FIXTURE_TEST_CASE(netifaddrs_move_assign_test, netifaddrs_fixture_2)
{
  nifs2_ = std::move(nifs1_);
  // nifs1_ should be empty and nifs2_ should at least have the loopback device
  BOOST_TEST(!nifs1_);
  BOOST_TEST(nifs2_);
}

/**
 * Test `netifaddrs_list` compatibility with `std::for_each`.
 *
 * Nothing is done as only iteration is tested.
 */
BOOST_FIXTURE_TEST_CASE(netifaddrs_iter_foreach_test, netifaddrs_fixture_1)
{
  std::for_each(std::begin(nifs_), std::end(nifs_), [](const auto&) {});
}

// input traits for the netifaddrs_list::iterator
using netifsaddrs_iter_traits_inputs = std::tuple<
  pdxka::is_indirectly_readable<pdxka::netifaddrs_list::iterator>,
  pdxka::is_equality_comparable<pdxka::netifaddrs_list::iterator>,
  pdxka::is_inequality_comparable<pdxka::netifaddrs_list::iterator>,
  pdxka::is_member_accessible<pdxka::netifaddrs_list::iterator>
>;

/**
 * Test that the `netifaddrs_list::iterator` satisfies the appropriate traits.
 *
 * @todo Increase number of tested traits when available.
 */
BOOST_AUTO_TEST_CASE_TEMPLATE(
  netifaddrs_iter_traits_test, T, netifsaddrs_iter_traits_inputs)
{
  BOOST_TEST(T::value, boost::core::demangle(typeid(T).name()) << "::value false");
}

BOOST_AUTO_TEST_SUITE_END()  // netif_test
