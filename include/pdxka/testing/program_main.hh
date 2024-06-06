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
#include <tuple>
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
   * @tparam Ns_... Char array sizes
   * @tparam Is... Sequential index values
   *
   * @param idxs Index sequence to deduce index parameter pack from
   * @param args... Parameter pack of null-terminated char arrays
   */
  template <std::size_t... Ns_, std::size_t... Is>
  void assign_buffers(
    std::index_sequence<Is...> /*idxs*/, const char (&...args)[Ns_]) noexcept
  {
    static_assert(sizeof...(Ns_) == sizeof...(Is));
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
template <std::size_t... Ns>
inline int program_main(
  testing::argument_vector<Ns...>& argv, const rss_provider& provider)
{
  return program_main(argv.argc(), argv.argv(), provider);
}

}  // namespace pdxka

#endif  // PDXKA_TESTING_PROGRAM_MAIN_HH_
