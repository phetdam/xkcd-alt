/**
 * @file testing/process.hh
 * @author Derek Huang
 * @brief C++ header for process management helpers
 * @copyright MIT License
 */

#ifndef PDXKA_TESTING_PROCESS_HH_
#define PDXKA_TESTING_PROCESS_HH_

#include <string>
#include <system_error>
#include <utility>

#include <boost/process.hpp>

namespace pdxka {
namespace testing {

/**
 * Class representing the output of a completed process.
 */
class process_output {
public:
  /**
   * Return the process's error code.
   */
  const auto& error_code() const noexcept
  {
    return error_code_;
  }

  /**
   * Return what was written by the process to standard output.
   */
  const auto& output() const noexcept
  {
    return output_;
  }

  /**
   * Return what was written by the process to standard error.
   */
  const auto& error_output() const noexcept
  {
    return error_output_;
  }

private:
  std::error_code error_code_;
  std::string output_;
  std::string error_output_;

  // run_process modifies state
  template <typename... Args>
  friend auto run_process(Args&&... args);
};

/**
 * Run a command as a child process and synchronously capture output.
 *
 * @tparam Args... Type parameter pack
 *
 * @param args Parameter pack of arguments for `boost::process::child`
 * @returns `process_output` object containing captured output and error code
 */
template <typename... Args>
auto run_process(Args&&... args)
{
  namespace bp = boost::process;
  // process output to populate
  process_output output;
  // streams for redirected stdout and stderr
  bp::ipstream out_stream;
  bp::ipstream err_stream;
  // invoke child process (absolute path so working directory is irrelevant)
  bp::child child{
    std::forward<Args>(args)...,
    bp::std_out > out_stream,
    bp::std_err > err_stream
  };
  // current stdout + stderr lines
  std::string out_line;
  std::string err_line;
  // synchronously collect output
  while (child.running(output.error_code_)) {
    if (std::getline(out_stream, out_line) && out_line.size())
      output.output_ += out_line;
    if (std::getline(err_stream, err_line) && err_line.size())
      output.error_output_ += err_line;
  }
  return output;
}

}  // namespace testing
}  // namespace pdxka

#endif  // PDXKA_TESTING_PROCESS_HH_
