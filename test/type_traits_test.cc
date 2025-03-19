/**
 * @file type_traits_test.cc
 * @author Derek Huang
 * @brief type_traits.hh unit tests
 * @copyright MIT License
 */

#include "pdxka/type_traits.hh"

#include <deque>
#include <ios>
#include <memory>
#include <string>
#include <tuple>
#include <vector>
#include <type_traits>
#include <typeinfo>  // for well-formed typeid() usage
#include <utility>

#include <boost/core/demangle.hpp>
#include <boost/test/unit_test.hpp>

#include "pdxka/common.h"

// TODO: consider making this part of features.h
#if PDXKA_CPLUSPLUS >= 202002L
#define PDXKA_HAS_CXX20 1
#else
#define PDXKA_HAS_CXX20 0
#endif  // PDXKA_CPLUSPLUS < 202002L

namespace {

/**
 * Traits test input for a unary traits type.
 *
 * @tparam Traits Traits type
 * @tparam T Input type
 * @tparam truth Truth value
 */
template <template <typename> typename Traits, typename T, bool truth>
struct traits_input {
  using traits_type = Traits<T>;
  using truth_type = std::bool_constant<truth>;
};

/**
 * Base class template that serves as a type wrapper.
 *
 * This is for convenience purposes when creating user-defined types.
 */
template <typename T>
class type_wrapper {
public:
  type_wrapper(const T& obj) : obj_{obj} {}
  type_wrapper(T&& obj) : obj_{std::move(obj)} {}
  const T& obj() const noexcept { return obj_; }
private:
  T obj_;
};

/**
 * Traits test input for `is_indirectly_readable`.
 *
 * @tparam T type
 * @tparam truth Truth value
 */
template <typename T, bool truth>
using indirectly_readable_input = traits_input<
  pdxka::is_indirectly_readable, T, truth
>;

/**
 * User-defined type that is indirectly readable.
 */
template <typename T>
class indirectly_readable_type : public type_wrapper<T> {
public:
  using type_wrapper<T>::type_wrapper;
  const T* operator*() const noexcept { return &this->obj(); }
};

/**
 * Traits test input for `is_equality_comparable`.
 *
 * @tparam T type
 * @tparam truth Truth value
 */
template <typename T, bool truth>
using equality_comparable_input = traits_input<
  pdxka::is_equality_comparable, T, truth
>;

/**
 * User-defined type that is equality comparable.
 */
template <typename T>
class equality_comparable_type : public type_wrapper<T> {
public:
  using type_wrapper<T>::type_wrapper;

  bool operator==(const equality_comparable_type& other) const
  {
    return this->obj() == other.obj();
  }
};

/**
 * Traits test input for `is_inequality_comparable`.
 *
 * @tparam T type
 * @tparam truth Truth value
 */
template <typename T, bool truth>
using inequality_comparable_input = traits_input<
  pdxka::is_inequality_comparable, T, truth
>;

/**
 * User-defined type that is inequality comparable.
 */
template <typename T>
class inequality_comparable_type : public type_wrapper<T> {
public:
  using type_wrapper<T>::type_wrapper;

  bool operator!=(const inequality_comparable_type& other) const
  {
    // note: under C++20 expression rewriting rules, if T implements operator==
    // but not operator!=, its operator== overloads are also considered
    return this->obj() != other.obj();
  }
};

/**
 * Traits test input for `is_member_accessible`.
 *
 * @tparam T type
 * @tparam truth Truth value
 */
template <typename T, bool truth>
using member_accessible_input = traits_input<
  pdxka::is_member_accessible, T, truth
>;

/**
 * User-defined type whose value can have its members accessed.
 */
template <typename T>
class member_accessible_type : public type_wrapper<T> {
public:
  using type_wrapper<T>::type_wrapper;
  const T* operator->() const noexcept { return &this->obj(); }
};

/**
 * Traits test input for `is_pre_incrementable`.
 *
 * @tparam T type
 * @tparam truth Truth value
 */
template <typename T, bool truth>
using pre_incrementable_input = traits_input<
  pdxka::is_pre_incrementable, T, truth
>;

/**
 * User-defined pre-incrementable type.
 */
template <typename T>
class pre_incrementable_type : public type_wrapper<T> {
public:
  using type_wrapper<T>::type_wrapper;
  auto& operator++() noexcept { return *this; }
};

/**
 * Traits test input for `is_post_incrementable`.
 *
 * @tparam T type
 * @tparam truth Truth value
 */
template <typename T, bool truth>
using post_incrementable_input = traits_input<
  pdxka::is_post_incrementable, T, truth
>;

/**
 * User-defined post-incrementable type.
 */
template <typename T>
class post_incrementable_type : public type_wrapper<T> {
public:
  using type_wrapper<T>::type_wrapper;
  auto operator++(int) const noexcept { return *this; }
};

}  // namespace

BOOST_AUTO_TEST_SUITE(type_traits_test)

// traits test input types
using traits_test_inputs = std::tuple<
  // is_indirectly_readable
  indirectly_readable_input<int, false>,
  indirectly_readable_input<std::string*, true>,
  indirectly_readable_input<const char*, true>,
  indirectly_readable_input<int**, true>,
  indirectly_readable_input<void*, false>,
  indirectly_readable_input<void**, true>,
  indirectly_readable_input<indirectly_readable_type<std::string>, true>,
  indirectly_readable_input<indirectly_readable_type<int>, true>,
  indirectly_readable_input<std::vector<double>::iterator, true>,
  indirectly_readable_input<std::deque<int>::iterator, true>,
  indirectly_readable_input<std::wstring, false>,
  // is_equality_comparable
  equality_comparable_input<int, true>,
  equality_comparable_input<double, true>,
  equality_comparable_input<void, false>,
  equality_comparable_input<std::string, true>,
  equality_comparable_input<std::vector<std::string>, true>,
  equality_comparable_input<void*, true>,
  equality_comparable_input<equality_comparable_type<unsigned>, true>,
  equality_comparable_input<equality_comparable_type<std::string>, true>,
  equality_comparable_input<indirectly_readable_type<int>, false>,
  equality_comparable_input<indirectly_readable_type<std::string>, false>,
  // is_inequality_comparable
  inequality_comparable_input<double, true>,
  inequality_comparable_input<std::string, true>,
  inequality_comparable_input<void*, true>,
  inequality_comparable_input<indirectly_readable_type<double>, false>,
  inequality_comparable_input<indirectly_readable_type<std::string>, false>,
  inequality_comparable_input<inequality_comparable_type<int>, true>,
  // under C++20 rewriting rules, if only T::operator== is available, for !=
  // expressions, operator== overloads are considered
  inequality_comparable_input<equality_comparable_type<unsigned>, PDXKA_HAS_CXX20>,
  // is_member_accessible
  member_accessible_input<std::unique_ptr<std::string>, true>,
  // note: operator-> not provided for T[] partial specialization
  member_accessible_input<std::unique_ptr<double[]>, false>,
  // but T need not have any accessible members
  member_accessible_input<std::unique_ptr<double>, true>,
  member_accessible_input<double, false>,
  member_accessible_input<std::pair<int, char>, false>,
  member_accessible_input<member_accessible_type<double>, true>,
  // is_pre_incrementable
  pre_incrementable_input<double, true>,
  pre_incrementable_input<std::deque<unsigned>, false>,
  pre_incrementable_input<const void*, false>,  // no void* arithmetic allowed
  pre_incrementable_input<pre_incrementable_type<std::deque<int>>, true>,
  pre_incrementable_input<pre_incrementable_type<double>, true>,
  // is_post_incrementable
  post_incrementable_input<double, true>,
  post_incrementable_input<std::string, false>,
  post_incrementable_input<std::deque<int>, false>,
  post_incrementable_input<post_incrementable_type<int>, true>,
  post_incrementable_input<post_incrementable_type<void*>, true>,
  post_incrementable_input<double**, true>
>;

/**
 * Test that the traits specializations have the expected truth value.
 */
BOOST_AUTO_TEST_CASE_TEMPLATE(traits_test, T, traits_test_inputs)
{
  // traits type and truth type
  using traits_type = typename T::traits_type;
  using truth_type = typename T::truth_type;
  // expected and actual truth values
  constexpr bool expected = truth_type::value;
  constexpr bool actual = traits_type::value;
  // check
  BOOST_TEST(
    expected == actual,
    boost::core::demangle(typeid(traits_type).name()) <<
      "::value != expected [" << std::boolalpha <<
      actual << " != " << expected << "]"
  );
}

BOOST_AUTO_TEST_SUITE_END()  // type_traits_test
