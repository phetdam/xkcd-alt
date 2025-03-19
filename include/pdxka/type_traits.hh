/**
 * @file type_traits.hh
 * @author Derek Huang
 * @brief C++ header for type traits
 * @copyright MIT License
 */

#ifndef PDXKA_TYPE_TRAITS_HH_
#define PDXKA_TYPE_TRAITS_HH_

#include <type_traits>
#include <utility>

namespace pdxka {

/**
 * Traits to check that a type is dereferenceable.
 *
 * @tparam T type
 */
template <typename T, typename = void>
struct is_indirectly_readable : std::false_type {};

/**
 * True specialization for dereferenceable types.
 *
 * @tparam T type
 */
template <typename T>
struct is_indirectly_readable<T, std::void_t<decltype(*std::declval<T>())> >
  : std::true_type {};

/**
 * Indicate that a type is dereferenceable.
 *
 * @tparam T type
 */
template <typename T>
constexpr bool is_indirectly_readable_v = is_indirectly_readable<T>::value;

/**
 * Traits to check if a type is equality comparable.
 *
 * @tparam T type
 */
template <typename T, typename = void>
struct is_equality_comparable : std::false_type {};

/**
 * True specialization for equality comparable types.
 *
 * @tparam T type
 */
template <typename T>
struct is_equality_comparable<
  T,
  std::void_t<decltype(std::declval<T>() == std::declval<T>())> >
  : std::true_type {};

/**
 * Indicate that a type is equality comparable.
 *
 * @tparam T type
 */
template <typename T>
constexpr bool is_equality_comparable_v = is_equality_comparable<T>::value;

/**
 * Traits to check if a type is inequality comparable.
 *
 * @tparam T type
 */
template <typename T, typename = void>
struct is_inequality_comparable : std::false_type {};

/**
 * True specialization for inequality comparable types.
 *
 * @tparam T type
 */
template <typename T>
struct is_inequality_comparable<
  T,
  std::void_t<decltype(std::declval<T>() != std::declval<T>())> >
  : std::true_type {};

/**
 * Indicate that a type is inequaltiy comparable.
 *
 * @tparam T type
 */
template <typename T>
constexpr bool is_inequality_comparable_v = is_inequality_comparable<T>::value;

/**
 * Traits to check that a type supports member access via `operator->`.
 *
 * This also works for fundamental types that implicitly define this operator.
 *
 * @tparam T type
 */
template <typename T, typename = void>
struct is_member_accessible : std::false_type {};

/**
 * True specialization for types that support the member access operator.
 *
 * @tparam T type
 */
template <typename T>
struct is_member_accessible<
  T,
  std::void_t<decltype(std::declval<T>().operator->())> > : std::true_type {};

/**
 * Indicate that a type supports member access through `operator->`.
 *
 * @tparam T type
 */
template <typename T>
constexpr bool is_member_accessible_v = is_member_accessible<T>::value;

/**
 * Traits to check that a type supports pre-increment.
 *
 * @tparam T type
 */
template <typename T, typename = void>
struct is_pre_incrementable : std::false_type {};

/**
 * True specialization for types that support pre-increment.
 *
 * @note We make use of reference collapsing rules here in the `declval<T>`
 *  expression. Note that pre-increment requires an lvalue.
 *
 * @tparam T type
 */
template <typename T>
struct is_pre_incrementable<T, std::void_t<decltype(++std::declval<T&>())> >
  : std::true_type {};

/**
 * Indicate that a type is pre-incrementable.
 *
 * @tparam T type
 */
template <typename T>
constexpr bool is_pre_incrementable_v = is_pre_incrementable<T>::value;

/**
 * Traits to check that a type supports post-increment.
 *
 * @tparam T type
 */
template <typename T, typename = void>
struct is_post_incrementable : std::false_type {};

/**
 * True specialization for types that support post-increment.
 *
 * @note As with pre-increment, post-increment is done on an lvalue.
 *
 * @tparam T type
 */
template <typename T>
struct is_post_incrementable<T, std::void_t<decltype(std::declval<T&>()++)> >
  : std::true_type {};

/**
 * Indicate that a type supports post-increment.
 *
 * @tparam T type
 */
template <typename T>
constexpr bool is_post_incrementable_v = is_post_incrementable<T>::value;

}  // namespace pdxka

#endif  // PDXKA_TYPE_TRAITS_HH_
