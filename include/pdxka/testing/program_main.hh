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
  static inline constexpr std::size_t n_args = sizeof...(Ns);
  static_assert(n_args > 0 && n_args < std::numeric_limits<int>::max());

  /**
   * Ctor.
   *
   * Copies the contents of the null-terminated char arrays or string literals,
   * including the null terminators, to the tuple member of the object.
   *
   * @param args... Parameter pack of null-terminated character arrays
   */
  argument_vector(const char (&...args)[Ns]) noexcept
  {
    assign_buffers(std::make_index_sequence<n_args>{}, args...);
  }

  /**
   * Return the number of arguments as a signed int for the `argv` of `main`.
   */
  static constexpr int argc()
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

  /**
   * Copy all null-terminated buffers into this object's internal buffers.
   *
   * @tparam Is... Sequential index values `0` through `sizeof...(Ns) - 1`
   *
   * @param idxs Index sequence to deduce index parameter pack from
   * @param args... Parameter pack of null-terminated char arrays
   */
  template <std::size_t... Is>
  void assign_buffers(
    std::index_sequence<Is...> /*idxs*/, const char (&...args)[Ns]) noexcept
  {
    // minimal check that packs are the same size
    static_assert(sizeof...(Is) == sizeof...(Ns));
    // fold to copy buffers + set pointers in argv_
    (
      [this, args]
      {
        std::memcpy(std::get<Is>(args_), args, Ns);
        argv_[Is] = std::get<Is>(args_);
      }()
      ,
      ...
    );
  }
};

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
