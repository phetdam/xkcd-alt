/**
 * @file pdxka_bpo_check.cc
 * @author Derek Huang
 * @brief C++ test checking whether built program uses Boost.ProgramOptions
 * @copyright MIT License
 */

#include <cstdlib>
#include <iostream>
#include <string>
#include <system_error>

#include <boost/process/child.hpp>
#include <boost/process/io.hpp>
#include <boost/process/pipe.hpp>
#include <boost/process/search_path.hpp>
#include <boost/timer/timer.hpp>

#include "pdxka/version.h"

// FIXME: these should be part of a testing header
#define STDERR_OUTPUT_BEGIN \
  "==BEGIN STDERR OUTPUT=======================================================\n"
#define STDERR_OUTPUT_END \
  "\n==END STDERR OUTPUT========================================================="

int main()
{
  namespace bp = boost::process;
  // timer for the current scope
  boost::timer::auto_cpu_timer scope_timer;
  // streams for redirected stdout and stderr
  bp::ipstream out_stream;
  bp::ipstream err_stream;
  // invoke child process
  // FIXME: only works if current directory is build directory or if the build
  // directory itself is on PATH. bake in the absolute path using a macro? we
  // weren't able to get relative path invocation (e.g. using ./) to work
  bp::child child{
    bp::search_path(PDXKA_PROGNAME), "-V",
    bp::std_out > out_stream,
    bp::std_err > err_stream
  };
  // synchronously collect output
  std::string out_line;    // stdout line
  std::string err_line;    // stderr line
  std::string output;      // stdout output
  std::string err_output;  // stderr output
  while (child.running()) {
    if (std::getline(out_stream, out_line) && out_line.size())
      output += out_line;
    if (std::getline(err_stream, err_line) && err_line.size())
      err_output += err_line;
  }
  // check for error
  if (err_output.size()) {
    std::cerr << "Error: Test produced output on stderr.\n" <<
      STDERR_OUTPUT_BEGIN << err_output << STDERR_OUTPUT_END << std::endl;
    return EXIT_FAILURE;
  }
  // search for Boost version
  // FIXME: all this does is echo the output
  std::cout << output << std::endl;
  return EXIT_SUCCESS;
}
