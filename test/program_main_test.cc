/**
 * @file program_main_test.cc
 * @author Derek Huang
 * @brief program_main.hh unit tests
 * @copyright MIT License
 */

#include "pdxka/program_main.hh"

#include <cstdlib>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <regex>
#include <sstream>
#include <string>
#include <tuple>
#include <type_traits>
#include <utility>
#include <vector>

#include <boost/test/data/test_case.hpp>
#include <boost/test/unit_test.hpp>
#include <curl/curl.h>

#include "pdxka/curl.hh"
#include "pdxka/testing/path.hh"
#include "pdxka/testing/program_main.hh"
#include "pdxka/testing/stream_diverter.hh"
#include "pdxka/version.h"

namespace pt = pdxka::testing;

namespace {

/**
 * Mock a network call retrieving the XKCD RSS XML content.
 *
 * This returns the XKCD RSS page retrieved on 2024/06/04.
 *
 * @param opts Ignored command-line options struct
 */
pdxka::curl_result mock_rss_get(const pdxka::cliopts& /*opts*/)
{
  // open stream to XML file content
  std::ifstream fs{(pt::data_dir() / "xkcd-rss-20240604.xml").string()};
  // read contents into string
  std::string rss_str;
  for (std::string line; std::getline(fs, line); )
    rss_str += line;
  // return new curl_result. move to avoid copy of rss_str
  return {CURLE_OK, "", pdxka::request_type::get, std::move(rss_str)};
}

}  // namespace

BOOST_AUTO_TEST_SUITE(program_main_test)

namespace {

/**
 * Traits helper that provides the expected exit code of the mocking test.
 *
 * If the provided type has an static `int` member called `exit_code` then its
 * value is used, otherwise the default `EXIT_SUCCESS` is used.
 *
 * @tparam T Input type
 */
template <typename T, typename = void>
struct exit_code_traits {
  static constexpr int value = EXIT_SUCCESS;
};

/**
 * Specialization for types that have the `exit_code` member.
 *
 * @tparam T Input type
 */
template <typename T>
struct exit_code_traits<
  T,
  std::enable_if_t<std::is_same_v<int, std::remove_cv_t<decltype(T::exit_code)>>>
> {
  static constexpr int value = T::exit_code;
};

/**
 * Helper type that provides an `EXIT_FAILURE` valued `exit_code` member.
 *
 * Input types can derive from this type to get the `exit_code` member.
 */
struct exit_failure_input {
  static constexpr int exit_code = EXIT_FAILURE;
};

/**
 * Traits helper that provides regular expressions to match for failure.
 *
 * This is similar to CMake's `FAIL_REGULAR_EXPRESSION`. If a match is made to
 * the output of the program with any of the regular expressions, test fails.
 * The input type can define its own static `fail_regexes` member that returns
 * a vector of `std::string` objects to do `regex_search` with.
 *
 * The default, as provided here, is to return an empty vector.
 *
 * @tparam T Input type
 */
template <typename T, typename = void>
struct fail_regex_traits {
  std::vector<std::string> operator()() const
  {
    return {};
  }
};

/**
 * Specialization for types that provide their own `fail_regexes` member.
 *
 * @tparam T Input type
 */
template <typename T>
struct fail_regex_traits<
  T,
  std::enable_if_t<
    std::is_same_v<std::vector<std::string>, decltype(T::fail_regexes())>>
> {
  auto operator()() const
  {
    return T::fail_regexes();
  }
};

/**
 * Helper type for a test that isn't supposed to access XKCD.
 *
 * The `--` and the beginning of the XKCD URL are used as a fail regex. This is
 * exactly what is used in `FAIL_REGULAR_EXPRESSION` for some CMake tests.
 */
struct no_xkcd_input {
  static std::vector<std::string> fail_regexes()
  {
    return {"-- https://xkcd.com"};
  }
};

/**
 * Callable object that returns the first `mock_program_main` input.
 *
 * This provides no arguments except the program name.
 */
struct argv_type_1 {
  auto operator()() const
  {
    // note: argument_vector CTAD fails with MSVC. see testing/program_main.hh
    // comments for why we have an extra function template for deduction
    return pt::make_argument_vector(PDXKA_PROGNAME);
  }
};

/**
 * Callable object that returns the second `mock_program_main` input.
 *
 * This requests an XKCD strip alt text 2 days previous from today.
 */
struct argv_type_2 {
  auto operator()() const
  {
    return pt::make_argument_vector(PDXKA_PROGNAME, "-b2");
  }
};

/**
 * Callable object that return the third `mock_program_main` input.
 *
 * This requests an XKCD strip alt text 3 days previous from today on one line.
 */
struct argv_type_3 {
  auto operator()() const noexcept
  {
    return pt::make_argument_vector(PDXKA_PROGNAME, "-o", "-b3");
  }
};

/**
 * Callable object that returns the first `mock_program_main` input.
 *
 * This requests an XKCD strip alt text 1 day previous from today while also
 * specifying this using `-b` with a separate value as an argument.
 */
struct argv_type_4 {
  auto operator()() const noexcept
  {
    return pt::make_argument_vector(PDXKA_PROGNAME, "-b", "1");
  }
};

/**
 * Callable object that returns the first `mock_program_main` input.
 *
 * This makes the 1 day previous alt text request using `--back`.
 */
struct argv_type_5 {
  auto operator()() const noexcept
  {
    return pt::make_argument_vector(PDXKA_PROGNAME, "--back");
  }
};

/**
 * Callable object that returns the second `mock_program_main` input.
 *
 * This makes the 2 day previous alt text request using `--back`.
 */
struct argv_type_6 {
  auto operator()() const noexcept
  {
    return pt::make_argument_vector(PDXKA_PROGNAME, "--back", "2");
  }
};

/**
 * Callable object that returns the third `mock_program_main` input.
 *
 * This makes the 3 day previous alt text request using `--back=3`.
 */
struct argv_type_7 {
  auto operator()() const noexcept
  {
    return pt::make_argument_vector(PDXKA_PROGNAME, "--back=3");
  }
};

/**
 * Callable object that errors because we backed up too much.
 */
struct argv_type_8 : exit_failure_input, no_xkcd_input {
  auto operator()() const noexcept
  {
    return pt::make_argument_vector(PDXKA_PROGNAME, "-b1000");
  }
};

/**
 * Callable object that errors because we backed up too much (long option).
 */
struct argv_type_9 : exit_failure_input, no_xkcd_input {
  auto operator()() const noexcept
  {
    return pt::make_argument_vector(PDXKA_PROGNAME, "--back", "8888");
  }
};

/**
 * Callable object that errors because we provided a negative value to -b.
 *
 * In this case it is interpreted as an unknown command line option.
 */
struct argv_type_10 : exit_failure_input, no_xkcd_input {
  auto operator()() const noexcept
  {
    return pt::make_argument_vector(PDXKA_PROGNAME, "-b", "-1900");
  }
};

/**
 * Callable object that errors because we provided a negative value to -b.
 *
 * In this case it is interpreted as an invalid negative value.
 */
struct argv_type_11 : exit_failure_input, no_xkcd_input {
  auto operator()() const noexcept
  {
    return pt::make_argument_vector(PDXKA_PROGNAME, "-b-9888");
  }
};

/**
 * Input type tuple for the `mock_program_main` test.
 */
using argv_type_tuple = std::tuple<
  argv_type_1,
  argv_type_2,
  argv_type_3,
  argv_type_4,
  argv_type_5,
  argv_type_6,
  argv_type_7,
  argv_type_8,
  argv_type_9,
  argv_type_10,
  argv_type_11
>;

}  // namespace

/**
 * Run the program main using the RSS mocker on different CLI arguments.
 */
BOOST_AUTO_TEST_CASE_TEMPLATE(mock_program_main, T, argv_type_tuple)
{
  // run program main with RSS mocker, redirecting output to stringstream
  // note: this practice is probably not thread-safe
  std::stringstream out;
  int ret;
  {
    pt::stream_diverter out_diverter{std::cout, out};
    pt::stream_diverter err_diverter{std::cerr, out};
    ret = pt::program_main(T{}(), mock_rss_get);
  }
  // lvalue for output string for std::regex_search
  auto out_str = out.str();
  // error code must match
  constexpr auto target_ret = exit_code_traits<T>::value;
  BOOST_TEST_REQUIRE(
    ret == target_ret,
    "exit code " << ret << " != target exit code " << target_ret <<
      ". output:\n" << out_str
  );
  // check if any failure regexes can be found
  for (const auto& re : fail_regex_traits<T>{}())
    BOOST_TEST(
      !std::regex_search(out_str, std::regex{re}),
      "matched failure regex '" << re << "' against output:\n" << out_str
    );
}

BOOST_AUTO_TEST_SUITE_END()  // program_main_test
