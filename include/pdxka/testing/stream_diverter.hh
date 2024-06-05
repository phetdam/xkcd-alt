/**
 * @file testing/stream_diverter.hh
 * @author Derek Huang
 * @brief C++ header for output stream write diversion
 * @copyright MIT License
 */

#ifndef PDXKA_TESTING_STREAM_DIVERTER_HH_
#define PDXKA_TESTING_STREAM_DIVERTER_HH_

#include <cstdio>
#include <exception>
#include <ostream>
#include <streambuf>
#include <string>

#include "pdxka/common.h"

namespace pdxka {
namespace testing {

/**
 * Context manager for diverting writes to one stream to another.
 *
 * @tparam CharT Char type
 * @tparam Traits Char traits type
 */
template <typename CharT, typename Traits = std::char_traits<CharT>>
class stream_diverter {
public:
  using stream_type = std::basic_ostream<CharT, Traits>;
  using buffer_type = std::basic_streambuf<CharT, Traits>;

  /**
   * Ctor.
   *
   * @param orig Original stream
   * @param target Target stream to divert writes to
   */
  stream_diverter(stream_type& orig, stream_type& target)
    : orig_{orig}, target_{target}, orig_buf_{orig_.rdbuf(target_.rdbuf())}
  {}

  /**
   * Dtor.
   *
   * Restores the original stream's stream buffer.
   */
  virtual ~stream_diverter()
  {
    // rdbuf() is allowed to throw implementation-defined exceptions
    try {
      orig_.rdbuf(orig_buf_);
    }
    // note: std::fprintf used since we don't want to throw more
    catch (std::exception& ex) {
      // TODO: consider using low-level boost/core/demangle.hpp interface
      std::fprintf(
        stderr, "%s: STL exception: %s\n", PDXKA_PRETTY_FUNCTION_NAME, ex.what()
      );
    }
    catch (...) {
      std::fprintf(stderr, "%s: unknown exception", PDXKA_PRETTY_FUNCTION_NAME);
    }
  }

  /**
   * Return reference to the original output stream.
   */
  auto& orig() noexcept
  {
    return orig_;
  }

  /**
   * Return reference to the target output stream.
   */
  auto& target() noexcept
  {
    return target_;
  }

  /**
   * Return pointer to original stream's stream buffer.
   */
  auto orig_buf() const noexcept
  {
    return orig_buf_;
  }

private:
  stream_type& orig_;
  stream_type& target_;
  buffer_type* orig_buf_;
};

}  // namespace testing
}  // namespace pdxka

#endif  // PDXKA_TESTING_STREAM_DIVERTER_HH_
