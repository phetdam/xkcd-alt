/**
 * @file com_test.cc
 * @author Derek Huang
 * @brief com.hh unit tests
 * @copyright MIT License
 */

#include "pdxka/com.hh"

#include <errhandlingapi.h>
#include <IMessageDispatcher.h>
#include <netlistmgr.h>
#include <servprov.h>
#include <Unknwn.h>  // IUnknown

#include <ios>
#include <string>
#include <tuple>
#include <type_traits>
#include <typeinfo>  // for well-formed typeid usage

#include <boost/test/unit_test.hpp>

BOOST_AUTO_TEST_SUITE(com_test)

/**
 * Check that `com_error` works as expected.
 */
BOOST_AUTO_TEST_CASE(com_error_test)
{
  // Win32 system error to set + expected and actual HRESULTs
  constexpr DWORD exp_err = ERROR_TOO_MANY_OPEN_FILES;
  // note: HRESULT_FROM_WIN32 was formerly a macro
  HRESULT exp_hres = HRESULT_FROM_WIN32(exp_err);
  HRESULT act_hres = S_OK;
  try {
    // set the per-thread error indicator with something
    SetLastError(exp_err);
    throw pdxka::com_error{"too many open files"};
  }
  catch (const pdxka::com_error& exc) {
    act_hres = exc.error();
  }
  // system errors must be the same
  BOOST_TEST(exp_hres == act_hres);
}

/**
 * COM type traits test input.
 *
 * Since the COM types are typically abstract they of course cannot be
 * instantiated. However, Boost.Test template test cases using
 * `BOOST_AUTO_TEST_CASE_TEMPLATE` result in an attempted instantiation of the
 * input tuple, which of course is not possible. This wrapper type holds the
 * input type as a type member and contains only a boolean member.
 *
 * @tparam T Input type
 * @tparam U Truth type, e.g. any type with a `value` boolean static member
 */
template <typename T, typename U>
struct traits_test_input {
  static_assert(std::is_convertible_v<decltype(U::value), bool>);
  using type = T;
  static constexpr bool value = U::value;
};

// input pairs for is_com_unknown_test
using is_com_unknown_test_inputs = std::tuple<
  traits_test_input<IUnknown, std::true_type>,
  traits_test_input<int, std::false_type>,
  traits_test_input<IServiceProvider, std::true_type>,
  traits_test_input<INetworkListManager, std::true_type>,
  traits_test_input<std::string, std::false_type>
>;

/**
 * Test that `is_com_unknown` works as expected.
 */
BOOST_AUTO_TEST_CASE_TEMPLATE(is_com_unknown_test, T, is_com_unknown_test_inputs)
{
  // input type
  using input_type = typename T::type;
  // expected and actual truth
  constexpr bool expected = T::value;
  constexpr bool actual = pdxka::is_com_unknown_v<input_type>;
  // check
  BOOST_TEST(
    expected == actual,
    "is_com_unknown_v<" << boost::core::demangle(typeid(input_type).name()) <<
      "> is " << std::boolalpha << actual << " != expected " << expected
  );
}

// input pairs for is_com_dispatch_test
// TODO: get more IDispatch subclasses
using is_com_dispatch_test_inputs = std::tuple<
  traits_test_input<std::string, std::false_type>,
  traits_test_input<INetworkListManager, std::true_type>,
  traits_test_input<IMessageDispatcher, std::false_type>,
  traits_test_input<IServiceProvider, std::false_type>
>;

/**
 * Test that `is_com_dispatch` works as expected.
 */
BOOST_AUTO_TEST_CASE_TEMPLATE(is_com_dispatch_test, T, is_com_dispatch_test_inputs)
{
  // input type
  using input_type = typename T::type;
  // expected and actual truth
  constexpr bool expected = T::value;
  constexpr bool actual = pdxka::is_com_dispatch_v<input_type>;
  // check
  BOOST_TEST(
    expected == actual,
    "is_com_dispatch_v<" << boost::core::demangle(typeid(input_type).name()) <<
      "> is " << std::boolalpha << actual << " != expected " << expected
  );
}

BOOST_AUTO_TEST_SUITE_END()  // com_test
