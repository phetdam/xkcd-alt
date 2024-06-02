/**
 * @file curl.hh
 * @author Derek Huang
 * @brief C++ header for libcurl wrappers and extensions
 * @copyright MIT License
 */

#ifndef PDXKA_CURL_HH_
#define PDXKA_CURL_HH_

#include <string>
#include <type_traits>

#include <curl/curl.h>

namespace pdxka {

/**
 * Macro evaluating to true if a `CURLcode` is `CURLE_OK`.
 *
 * @param status `CURLcode` cURL status code
 */
#define PDXKA_CURL_OK(status) if ((status) == CURLE_OK)

/**
 * Macro evaluating to true if a `CURLcode` is not `CURLE_OK`.
 *
 * @param status `CURLcode` cURL status code
 */
#define PDXKA_CURL_NOT_OK(status) if ((status) != CURLE_OK)

/**
 * Macro error handler for cURL when a `CURLcode` is not `CURLE_OK`.
 *
 * The value of `errbuf` is assigned to `reason` and the code will goto the
 * `cleanup` label to execute whatever cleanup code is necessary.
 *
 * @param status `CURLcode` cURL status code
 * @param reason `std::string` to write cURL error buffer to
 * @param errbuf `char*` or `char[]` cURL error buffer ending in `NULL`
 * @param cleanup `goto` label to jump to for cleanup
 */
#define PDXKA_CURL_ERR_HANDLER(status, reason, errbuf, cleanup) \
  do { \
    PDXKA_CURL_NOT_OK(status)  { \
      reason = errbuf; \
      goto cleanup; \
    } \
  } \
  while (true)

/**
 * Enum class for the `curl_result` HTTP[S] request type.
 */
enum class request_type { GET, POST };

/**
 * Immutable POD class holding cURL HTTP[S] result.
 *
 * @param status `CURLcode` cURL status code
 * @param reason `std::string` holding contents of last cURL error buffer
 * @param request `request_type` HTTP[S] request we got result for
 * @param payload `std::string` HTTP[S] response body
 */
struct curl_result {
  const CURLcode status;
  const std::string reason;
  const request_type request;
  const std::string payload;
};

/**
 * Template class holding cURL options.
 *
 * @tparam T cURL option value type
 */
template <typename T>
class curl_option {
public:
  using value_type = T;

  /**
   * Constructor.
   *
   * @param name `CURLoption` cURL option enum value
   * @param value `T` cURL option value to set
   */
  curl_option(CURLoption name, T value) : name_(name), value_(value) {}

  /**
   * Return cURL option enum value.
   */
  auto name() const noexcept
  {
    return name_;
  }

  /**
   * Return the option value.
   *
   * This is by const ref if the type is larger than a pointer else by value.
   */
  std::conditional_t<sizeof(T) <= sizeof(void*), T, const T&> value() const noexcept
  {
    return value_;
  }

private:
  CURLoption name_;
  T value_;
};

}  // namespace pdkxa

#endif  // PDXKA_CURL_HH_
