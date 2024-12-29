/**
 * @file testing/program_main.hh
 * @author Derek Huang
 * @brief C++ header with test helpers for program_main.hh
 * @copyright MIT License
 */

#ifndef PDXKA_TESTING_PROGRAM_MAIN_HH_
#define PDXKA_TESTING_PROGRAM_MAIN_HH_

#include <cstdint>
#include <cstring>
#include <functional>
#include <limits>
#include <ostream>
#include <tuple>
#include <type_traits>
#include <utility>

#include "pdxka/program_main.hh"

namespace pdxka {

namespace testing {

/**
 * Compile-time argument vector for representing `argc` and `argv`.
 *
 * This class provides a convenient way to mock the values of `argc` and `argv`
 * needed by `main` or by `pdxka::program_main`.
 *
 * @tparam Ns... Null-terminated char array sizes
 */
template <std::size_t... Ns>
class argument_vector {
public:
  /**
   * Argument count including the program name.
   *
   * This is guaranteed to be positive and not overflow a signed integer.
   */
  static constexpr auto n_args = sizeof...(Ns);
  static_assert(n_args > 0 && n_args < std::numeric_limits<int>::max());

#ifndef _MSC_VER
  /**
   * Ctor.
   *
   * Copies the contents of the null-terminated char arrays or string literals,
   * including the null terminators, to the tuple member of the object.
   *
   * @todo This does not compile with Visual Studio 2022 as CTAD fails and for
   *  some reason we get a C3520 complaining about Ns not being expanded.
   *
   * @param args... Parameter pack of null-terminated character arrays
   */
  argument_vector(const char (&...args)[Ns]) noexcept
  {
    assign_buffers(std::make_index_sequence<n_args>{}, args...);
  }
#endif  // _MSC_VER

  /**
   * Copy ctor.
   *
   * We need a dedicated copy ctor because none of the class members are
   * copy-constructible. In particular, using the defaulted copy ctor of a
   * tuple with non-copy-constructible members, specifically where
   * `std::is_copy_constructible_v<T>` is false for each `T` in the tuple, is
   * UB until C++20 and considered ill-formed from C++20 onwards.
   *
   * Furthermore, the `argv()` pointers need to refer to the data items in the
   * current argument vector object and thus need to be appropriately set.
   * Otherwise, if they refer to the original object's data, we then have an
   * object lifetime issue and copy behavior will be incorrect.
   *
   * @note A move ctor is unnecessary since this class's members are all POD.
   *
   * @note With Visual Studio 2022, if not compiling with `/permissive-`, this
   *  copy ctor is actually called if taking a prvalue as an rvalue reference
   *  into a function. It is possible a "move" is done, which in this case
   *  decays to copy since there is no user-defined move ctor.
   *
   * @param other Argument vector to copy from
   */
  argument_vector(const argument_vector& other) noexcept
  {
    assign_buffers(std::make_index_sequence<n_args>{}, other);
  }

  /**
   * Return the number of arguments as a signed int for the `argv` of `main`.
   */
  static constexpr int argc() noexcept
  {
    return n_args;
  }

  /**
   * Return a pointer to pointer to `char` for the `argv` of `main`.
   *
   * Each `char*` dereferencable from `argv()` points to a null-terminated char
   * buffer with lifetime scoped to this object's lifetime.
   */
  auto argv() noexcept
  {
    return argv_;
  }

  /**
   * Return a reference to the tuple that holds all the argument buffers.
   */
  auto& args() noexcept
  {
    return args_;
  }

  /**
   * Implicit conversion to tuple of char arrays for `std::get` interop.
   */
  operator auto&() noexcept
  {
    return args_;
  }

private:
  std::tuple<char[Ns]...> args_;
  char* argv_[n_args];

#ifdef _MSC_VER
  /**
   * Private default ctor.
   *
   * We only allow default construction via `make_argument_vector` for MSVC.
   */
  argument_vector() = default;
#endif  // _MSC_VER

  /**
   * Copy all null-terminated buffers into this object's internal buffers.
   *
   * @tparam Is... Sequential index values `0` through `sizeof...(Ns) - 1`
   * @tparam Ss... Null-terminated char array sizes
   *
   * @note The `Ss...` pack is introduced to appease MSVC (it does not like a
   *  "fixed" pack of non-type template parameters for some reason). For
   *  GCC/Clang even under -pedantic using `Ns` is sufficient.
   *
   * @param idxs Index sequence to deduce index parameter pack from
   * @param args... Parameter pack of null-terminated char arrays
   */
  template <std::size_t... Is, std::size_t... Ss>
  void assign_buffers(
    std::index_sequence<Is...> /*idxs*/, const char (&...args)[Ss]) noexcept
  {
    // minimal check that packs are the appropriate size
    static_assert(sizeof...(Is) == n_args);
    static_assert(sizeof...(Ss) == n_args);
    // fold to copy buffers + set pointers in argv_
    (
      [this, args]
      {
        std::memcpy(std::get<Is>(args_), args, Ss);
        argv_[Is] = std::get<Is>(args_);
      }()
      ,
      ...
    );
  }

  /**
   * Assign the object's buffers from another argument vector instance.
   *
   * @tparam Is... Sequential index values `0` through `sizeof...(Ns) - 1`
   * @tparam Ss... Null-terminated char array sizes
   *
   * @param idxs Index sequence to deduce index parameter pack from
   * @param other Argument vector to perform copy assignment from
   */
  template <std::size_t... Is, std::size_t... Ss>
  void assign_buffers(
    std::index_sequence<Is...> /*idxs*/,
    const argument_vector<Ss...>& other) noexcept
  {
    // minimal check that packs are the appropriate size
    static_assert(sizeof...(Is) == n_args);
    static_assert(sizeof...(Ss) == n_args);
    // fold to copy buffers + set pointers in argv_
    // note: cannot use defaulted tuple copy ctor since the members are arrays
    // and is_copy_constructible_v will be false for all the members
    (
      [this, &other]
      {
        std::memcpy(std::get<Is>(args_), std::get<Is>(other.args_), Ss);
        argv_[Is] = std::get<Is>(args_);
      }()
      ,
      ...
    );
  }

  // factory function friend
  template <std::size_t... Ns_>
  friend auto make_argument_vector(const char (&...args)[Ns_]) noexcept;
};

// TODO: document better
// creates an argument vector. this satisfies MSVC as CTAD oddly fails.
template <std::size_t... Ns>
auto make_argument_vector(const char (&...args)[Ns]) noexcept
{
// under MSVC, default-construct, and use friend access
#if defined(_MSC_VER)
  argument_vector<Ns...> vec;
  vec.assign_buffers(std::make_index_sequence<sizeof...(Ns)>{}, args...);
  return vec;
// otherwise, just use ctor
#else
  return argument_vector<Ns...>(args...);
#endif  // !defined(_MSC_VER)
}

namespace detail {

/**
 * Write an `argument_vector` to a stream.
 *
 * @tparam I First index value (must be zero)
 * @tparam Is... Sequential index values `1` through `sizeof...(Ns) - 1`
 * @tparam N First null-terminated char array size
 * @tparam Ns... Subsequent null-terminated char array sizes
 *
 * @param out Stream to write to
 * @param idxs Index sequence to deduce index parameter pack from
 * @param argv Argument vector constructed from null-terminated char arrays
 */
template <std::size_t I, std::size_t... Is, std::size_t N, std::size_t... Ns>
auto& write(
  std::ostream& out,
  std::index_sequence<I, Is...> /*idxs*/,
  argument_vector<N, Ns...>& argv)
{
  // minimal check that packs are the same size
  static_assert(I == 0);
  static_assert(sizeof...(Is) == sizeof...(Ns));
  // reference to argv's internal tuple
  auto& buffers = argv.args();
  // print argc + argv
  out << "argc=" << argv.argc() << ", argv=[\"" << std::get<I>(buffers) << "\"";
  (
    [&out, &buffers]
    {
      out << ", \"" << std::get<Is>(buffers) << "\"";
    }()
    ,
    ...
  );
  // done, just need to close bracket
  return out << "]";
}

}  // namespace detail

/**
 * `operator<<` overload for writing to a stream.
 *
 * This is also convenient for use with Boost.Test in lieu of using the
 * `boost_test_print_type` customization point provided.
 *
 * @tparam Ns... Null-terminated char array sizes
 *
 * @param out Stream to write to
 * @param argv Argument vector constructed from null-terminated char arrays
 */
template <std::size_t... Ns>
inline auto& operator<<(std::ostream& out, argument_vector<Ns...>& argv)
{
  return detail::write(out, std::make_index_sequence<sizeof...(Ns)>{}, argv);
}

/**
 * Traits struct to determine if a type is an `argument_vector` specialization.
 *
 * @tparam T type
 */
template <typename T>
struct is_argument_vector : std::false_type {};

/**
 * Partial specialization when the type is an `argument_vector` specialization.
 *
 * @tparam Ns... Null-terminate char array sizes
 */
template <std::size_t... Ns>
struct is_argument_vector<argument_vector<Ns...>> : std::true_type {};

/**
 * Helper value to determine if a type is an `argument_vector` specialization.
 *
 * @tparam T type
 */
template <typename T>
inline constexpr bool is_argument_vector_v = is_argument_vector<T>::value;

}  // namespace testing

/**
 * `pdxka` CLI tool program main convenience overload.
 *
 * This facilitates testing by allowing use of the `testing::argument_vector`.
 *
 * @tparam Ns... Null-terminated char array sizes
 *
 * @param argv Argument vector constructed from null-terminated char arrays
 * @param provider Callable providing the RSS XML to parse
 * @returns `EXIT_SUCCESS` on success, `EXIT_FAILURE` or higher or failure
 */
template <typename T>
inline
std::enable_if_t<testing::is_argument_vector_v<std::remove_reference_t<T>>, int>
program_main(T&& argv, const rss_provider& provider)
{
  return program_main(argv.argc(), argv.argv(), provider);
}

}  // namespace pdxka

#endif  // PDXKA_TESTING_PROGRAM_MAIN_HH_
