/**
 * @file pdxka_inet_test.cc
 * @author Derek Huang
 * @brief C++ program to check if Intener connection is available
 * @copyright MIT License
 */

#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif  // WIN32_LEAN_AND_MEAN
#include <Windows.h>
#include <combaseapi.h>
#include <objbase.h>
#include <netlistmgr.h>

#include <cstdlib>
#include <ios>
#include <iostream>
#include <string>

#include "pdxka/com.hh"

/**
 * @file
 *
 * This program is used as a CTest test fixture to skip any network-accessing
 * tests if there is no Internet connection. Therefore, when appropriate, tests
 * that access the network are not run (without the delay of retry/timeout
 * loops), but any network errors will still be caught when there is Internet.
 *
 * @note Currently this is only available for Windows.
 *
 * @todo Add some more COM abstraction and write a nicer program for POSIX.
 */

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

int main()
{
  // single-threaded COM initialization
  pdxka::coinit_context com{COINIT_APARTMENTTHREADED};
  // create network list manager instance
  pdxka::com_ptr<INetworkListManager> mgr;
  // check if connected + print message
  auto have_inet = has_inet_connection(*mgr);
  std::cout << "Internet: " << (have_inet ? "Yes" : "No") << std::endl;
  return have_inet ? EXIT_SUCCESS : EXIT_FAILURE;
}
