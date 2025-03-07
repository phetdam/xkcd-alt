/**
 * @file pdxka_inet_test.cc
 * @author Derek Huang
 * @brief C++ program to check if Intener connection is available
 * @copyright MIT License
 *
 * @file
 *
 * This program is used with the pdxka_live_test.cmake CMake script as a
 * pre-test step to skip launching any tests of xkcd-alt that will make network
 * connection if there is "no Internet". Therefore, when appropriate, these
 * tests are marked as skipped without the delay of retry/timeout loops.
 */

#if defined(_WIN32)
#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif  // WIN32_LEAN_AND_MEAN
#include <Windows.h>
#include <objbase.h>
#include <netlistmgr.h>
#else
#include <net/if.h>
#include <sys/socket.h>
#endif  // !defined(_WIN32)

#include <cstdlib>
#include <iostream>

#if defined(_WIN32)
#include "pdxka/com.hh"
#else
#include "pdxka/netif.hh"
#endif  // !defined(_WIN32)

namespace {

#if defined(_WIN32)
/**
 * Indicate whether or not the local machine is connected to Internet.
 */
bool has_inet_connection(INetworkListManager& mgr)
{
  // get status and value
  VARIANT_BOOL res;
  auto hres = mgr.get_IsConnectedToInternet(&res);
  if (FAILED(hres))
    throw pdxka::com_error{hres, "get_IsConnectedToInternet failed"};
  // convert to actual bool
  return res == VARIANT_TRUE;
}
#else
/**
 * Check if the local machine is connected to the Internet.
 *
 * That is, we check if there is at least one non-loopback network interface
 * that has an IPv4 or IPv6 address for Internet connection, i.e. `AF_INET` or
 * `AF_INET6`, that is actively running, ie. with `IFF_UP`.
 *
 * @param netifs List of network interface addresses
 */
bool has_inet(const pdxka::netifaddrs_list& netifs) noexcept
{
  for (const auto& nif : netifs) {
    // address family/socket domain + network device flags
    auto family = nif.ifa_addr->sa_family;
    auto flags = nif.ifa_flags;
    // if criteria matches, done
    if (
      (flags & IFF_UP) &&                        // interface running
      !(flags & IFF_LOOPBACK) &&                 // not a loopback address
      (family == AF_INET || family == AF_INET6)  // IPv4 or IPv6
    )
      return true;
  }
  return false;
}
#endif  // !defined(_WIN32)

}  // namespace

int main()
{
#if defined(_WIN32)
  // single-threaded COM initialization
  pdxka::coinit_context com{COINIT_APARTMENTTHREADED};
  // create network list manager instance
  pdxka::com_ptr<INetworkListManager> mgr;
  // check if connected + print message
  auto have_inet = has_inet_connection(*mgr);
#else
  // get list of network interfaces
  pdxka::netifaddrs_list netifs;
  // check if connected + print message
  auto have_inet = has_inet(netifs);
#endif  // !defined(_WIN32)
  std::cout << "Internet: " << (have_inet ? "Yes" : "No") << std::endl;
  return have_inet ? EXIT_SUCCESS : EXIT_FAILURE;
}
