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

int main()
{
  // single-threaded COM initialization
  pdxka::coinit_context com{COINIT_APARTMENTTHREADED};
  // create network list manager instance
  pdxka::com_ptr<INetworkListManager> mgr;
  // check if we have connection to Internet
  bool have_inet;
  // use lambda to get result and release reference in scope
  if (
    [&have_inet, mgr]
    {
      // get status and value
      VARIANT_BOOL res;
      auto err = mgr->get_IsConnectedToInternet(&res);
      // convert to bool + check status
      have_inet = (res == VARIANT_TRUE);
      return FAILED(err);
    }()
  ) {
    std::cerr << "Error: get_IsConnectedToInternet failed: HRESULT " <<
      std::hex << HRESULT_FROM_WIN32(GetLastError()) << std::endl;
    return EXIT_FAILURE;
  }
  // check if connected + print message
  std::cout << "Internet: " << (have_inet ? "Yes" : "No") << std::endl;
  return have_inet ? EXIT_SUCCESS : EXIT_FAILURE;
}
