/**
 * @file com_test.cc
 * @author Derek Huang
 * @brief com.hh unit tests
 * @copyright MIT License
 */

#include "pdxka/com.hh"

#include <ComSvcs.h>
#include <IMessageDispatcher.h>
#include <netlistmgr.h>
#include <servprov.h>
#include <Unknwn.h>  // IUnknown
#include <wincodec.h>

#include <ios>
#include <string>
#include <tuple>
#include <type_traits>
#include <typeinfo>  // for well-formed typeid usage
#include <utility>

#include <boost/test/unit_test.hpp>

namespace utf = boost::unit_test;

namespace {

/**
 * Fixture to ensure that COM is initialized for this thread.
 */
struct com_init_fixture {
private:
  pdxka::coinit_context ctx_;
};

}  // namespace

BOOST_AUTO_TEST_SUITE(com_test, * utf::fixture<com_init_fixture>())

/**
 * Check that `com_error` works as expected.
 */
BOOST_AUTO_TEST_CASE(com_error_test)
{
  using pdxka::com_error;
  // expected and actual HRESULTs
  // note: HRESULT_FROM_WIN32 was formerly a macro
  constexpr HRESULT exp_hres = E_NOINTERFACE;
  HRESULT act_hres = S_OK;
  try {
    throw com_error{exp_hres};
  }
  catch (const com_error& exc) {
    act_hres = exc.error();
  }
  // value should have changed
  BOOST_TEST_REQUIRE(act_hres != S_OK);
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
  traits_test_input<std::string, std::false_type>,
  traits_test_input<ISecurityCallContext, std::true_type>,
  traits_test_input<ISharedProperty, std::true_type>,
  traits_test_input<IWICImagingFactory, std::true_type>
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
  traits_test_input<IServiceProvider, std::false_type>,
  traits_test_input<ITransactionContext, std::true_type>,
  traits_test_input<IWICImagingFactory, std::false_type>
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

/**
 * Type wrapper object.
 *
 * This holds any type, even incomplete, in a regular type wrapper. In
 * particular, this type is helpful with `BOOST_AUTO_TEST_CASE_TEMPLATE` as
 * incomplete types in the input tuple will result in a compile error.
 *
 * @tparam T type
 */
template <typename T>
struct type_wrapper {
  using type = T;
};

// IUnknown types for com_ptr semantic tests
// note: these COM classes should be registered for most systems
using com_ptr_sem_test_inputs = std::tuple<
  type_wrapper<INetworkListManager>,
  type_wrapper<IWICImagingFactory>,
  type_wrapper<ITransactionContext>
>;

/**
 * Test that `com_ptr` copy works correctly.
 */
BOOST_AUTO_TEST_CASE_TEMPLATE(com_ptr_copy_test, T, com_ptr_sem_test_inputs)
{
  pdxka::com_ptr<typename T::type> o1;
  // copy + test
  auto o2 = o1;
  BOOST_TEST(o1);
  BOOST_TEST(o2);
  BOOST_TEST(o1 == o2);  // should have same data pointer
}

/**
 * Test that `com_ptr` move works correctly.
 */
BOOST_AUTO_TEST_CASE_TEMPLATE(com_ptr_move_test, T, com_ptr_sem_test_inputs)
{
  pdxka::com_ptr<typename T::type> o1;
  // move + test
  auto o2 = std::move(o1);
  BOOST_TEST(!o1);
  BOOST_TEST(o2);
}

/**
 * Test that `com_ptr` copy assignment works correctly.
 */
BOOST_AUTO_TEST_CASE_TEMPLATE(com_ptr_copy_assign_test, T, com_ptr_sem_test_inputs)
{
  pdxka::com_ptr<typename T::type> o1, o2;
  // copy assign + test
  o2 = o1;
  BOOST_TEST(o1);
  BOOST_TEST(o2);
  BOOST_TEST(o1 == o2);
}

/**
 * Test that `com_ptr` move assignment works correctly.
 */
BOOST_AUTO_TEST_CASE_TEMPLATE(com_ptr_move_assign_test, T, com_ptr_sem_test_inputs)
{
  pdxka::com_ptr<typename T::type> o1, o2;
  // move assign + test
  o2 = std::move(o1);
  BOOST_TEST(!o1);
  BOOST_TEST(o2);
}

BOOST_AUTO_TEST_SUITE_END()  // com_test
