/**
 * @file posix.hh
 * @author Derek Huang
 * @brief C++ header for POSIX helpers
 * @copyright MIT License
 */

#include <cerrno>
#include <cstring>
#include <stdexcept>
#include <string>

#ifndef PDXKA_POSIX_HH_
#define PDXKA_POSIX_HH_

namespace pdxka {

/**
 * Exception type to indicate a POSIX function has failed and set `errno`.
 */
class posix_error : public std::runtime_error {
public:
  /**
   * Default ctor.
   *
   * The current value of `errno` is used to construct the exception message.
   */
  posix_error() : posix_error{errno} {}

  /**
   * Ctor.
   *
   * @param err `errno` status
   */
  posix_error(int err)
    : std::runtime_error{"Error: " + std::string{std::strerror(err)}},
      err_{err}
  {}

  /**
   * Ctor.
   *
   * @param err `errno` status
   * @param message Message
   */
  posix_error(int err, const std::string& message)
    : std::runtime_error{"Error: " + message + ": " + std::strerror(err)},
      err_{err}
  {}

  /**
   * Return the `errno` value used in the exception.
   */
  auto err() const noexcept { return err_; }

  /**
   * Return the string message corresponding to the `errno` value.
   */
  const char* errmsg() const noexcept
  {
    return std::strerror(err_);
  }

private:
  int err_;
};

}  // namespace pdxka

#endif  // PDXKA_POSIX_HH_
