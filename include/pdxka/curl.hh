/**
 * @file curl.hh
 * @author Derek Huang
 * @brief C++ header for libcurl wrappers and extensions
 * @copyright MIT License
 */

#ifndef PDXKA_CURL_HH_
#define PDXKA_CURL_HH_

#include <cstdint>
#include <iostream>
#include <sstream>
#include <stdexcept>
#include <string>
#include <type_traits>
#include <utility>

#include <boost/exception/diagnostic_information.hpp>
#include <curl/curl.h>

#include "pdxka/common.h"
#include "pdxka/curl_traits.hh"
#include "pdxka/warnings.h"

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
  while (false)

/**
 * Enum class for the `curl_result` HTTP[S] request type.
 */
enum class request_type { get, post };

/**
 * Struct holding cURL HTTP[S] result.
 *
 * @param status `CURLcode` cURL status code
 * @param reason `std::string` holding contents of last cURL error buffer
 * @param request `request_type` HTTP[S] request we got result for
 * @param payload `std::string` HTTP[S] response body
 */
struct curl_result {
  CURLcode status;
  std::string reason;
  request_type request;
  std::string payload;
};

/**
 * Template class holding cURL options.
 *
 * @tparam O cURL option value
 */
template <CURLoption O>
class curl_option {
public:
  using value_type = curl_option_value_type<O>;

  /**
   * Constructor.
   *
   * @param value cURL option value to set
   */
  constexpr curl_option(value_type value) noexcept(noexcept(std::move(value)))
    : value_{std::move(value)}
  {}

  /**
   * Return cURL option enum value.
   */
  static constexpr auto name() noexcept
  {
    return O;
  }

  /**
   * Return the option value.
   *
   * Since the type is typically small we simply return by value.
   */
  constexpr auto value() const noexcept
  {
    return value_;
  }

private:
  value_type value_;
};

/**
 * Exception type associated with cURL errors.
 *
 * This will contain the cURL error code and a pointer to the error text.
 */
class curl_error : public std::runtime_error {
public:
  /**
   * Ctor.
   *
   * Construct with the cURL status and user-defined message. The call site is
   * specified as `"(unknown)"` since it is not provided.
   *
   * @param status cURL error status
   * @param message User-defined error message
   */
  curl_error(CURLcode status, const std::string& message)
    : curl_error("(unknown)", status, message)
  {}

  /**
   * Ctor.
   *
   * Construct with a user-specified call site, cURL status, and message.
   *
   * @param site Call site, e.g. `PDXKA_PRETTY_FUNCTION_NAME`
   * @param status cURL error status
   * @param message User-defined error message
   */
  curl_error(const char* site, CURLcode status, const std::string& message)
    : std::runtime_error{
        std::string{site} + ": " + message + ": " + curl_easy_strerror(status)
      },
      status_{status},
      error_text_{curl_easy_strerror(status)}
  {}

  /**
   * Return the cURL status code.
   */
  auto status() const noexcept { return status_; }

  /**
   * Return the cURL status error text.
   */
  auto error_text() const noexcept { return error_text_; }

private:
  CURLcode status_;
  const char* error_text_;
};

/**
 * Helper macro to throw a `curl_error` with the function signature.
 *
 * @param status cURL error status
 * @param message User-defined error message
 */
#define PDXKA_CURL_ERROR_THROW(status, message) \
  throw pdxka::curl_error{PDXKA_PRETTY_FUNCTION_NAME, status, message}

/**
 * Perform global libcurl initialization in a thread-safe manner.
 *
 * This will call `curl_global_init` and under C++11 is thread-safe. After the
 * first call, all other calls to this function will be no-ops.
 *
 * @param flags libcurl global initialization flags, e.g. `CURL_GLOBAL_DEFAULT`
 * @throws std::runtime_error If libcurl global initialization fails
 */
inline void init_curl(long flags = CURL_GLOBAL_DEFAULT)
{
  /**
   * libcurl initialization class.
   */
  class curl_loader {
  public:
    /**
     * Ctor.
     *
     * @param flags libcurl global initialization flags
     */
    curl_loader(long flags)
    {
      CURLcode status;
      PDXKA_CURL_NOT_OK(status = curl_global_init(flags))
        PDXKA_CURL_ERROR_THROW(status, "libcurl initialization failed");
    }

    /**
     * Dtor.
     *
     * This calls `curl_global_cleanup` to clean up resources.
     */
    ~curl_loader()
    {
      curl_global_cleanup();
    }
  };

  // under C++11 this is thread-safe
  static curl_loader loader{flags};
}

/**
 * Class wrapper for a libcurl easy handle with unique ownership.
 *
 * Global libcurl initialization is handled in a thread-safe manner.
 */
class curl_handle {
public:
// silence MSVC C4706 on assignment in curl_handle conditional
PDXKA_MSVC_WARNING_PUSH()
PDXKA_MSVC_WARNING_DISABLE(4706)
  /**
   * Ctor.
   *
   * Performs thread-safe libcurl global init if necessary and provides the
   * raw `CURL*` easy handle for use with libcurl.
   *
   * @throw std::runtime_error If libcurl init or `curl_easy_init` fails
   */
  curl_handle()
  {
    // perform thread-safe libcurl global init (no-op if already initialized)
    init_curl();
    // get new easy handle. if nullptr, error
    // note: although there is no corresponding CURLcode and cURL does not have
    // a per-thread error indicator, CURLE_FAILED_INIT is close enough to this
    if (!(handle_ = curl_easy_init()))
PDXKA_MSVC_WARNING_POP()
      PDXKA_CURL_ERROR_THROW(CURLE_FAILED_INIT, "curl_easy_init errored");
  }

  /**
   * Deleted copy ctor.
   */
  curl_handle(const curl_handle&) = delete;

  /**
   * Move ctor.
   *
   * @param other Handle to transfer ownership from
   */
  curl_handle(curl_handle&& other) noexcept
  {
    move(std::move(other));
  }

  /**
   * Move assignment operator.
   *
   * @param other Handle to transfer ownership from
   */
  auto& operator=(curl_handle&& other) noexcept
  {
    destroy_handle();
    move(std::move(other));
    return *this;
  }

  /**
   * Dtor.
   *
   * Clean up the easy handle if it is still owned.
   */
  ~curl_handle()
  {
    destroy_handle();
  }

  /**
   * Return the raw `CURL*` libcurl easy handle.
   */
  auto handle() const noexcept
  {
    return handle_;
  }

  /**
   * Return the raw `CURL*` libcurl easy handle for C function interop.
   */
  operator CURL*() const noexcept
  {
    return handle_;
  }

  /**
   * Set the cURL option on the handle with the given value.
   *
   * Throwing occurs only if setting the option can fail and if failure occurs.
   *
   * @tparam O cURL option value
   *
   * @param value cURL option argument value
   * @returns `*this` to allow method chaining
   */
  template <CURLoption O>
  auto& option(curl_option_value_type<O> value) & noexcept(curl_option_always_ok<O>)
  {
    set<O>(std::move(value));
    return *this;
  }

  /**
   * Set the cURL option on the handle with the given value.
   *
   * Throwing occurs only if setting the option can fail and if failure occurs.
   *
   * @tparam O cURL option value
   *
   * @param value cURL option argument value
   * @returns `std::move(*this)` to allow method chaining
   */
  template <CURLoption O>
  auto&& option(curl_option_value_type<O> value) && noexcept(curl_option_always_ok<O>)
  {
    set<O>(std::move(value));
    // by performing a move instead of returning *this we provide a fluent API
    // that allows creation via curl_handle{}.option<O>(value)
    return std::move(*this);
  }

  /**
   * Set the cURL option on the handle.
   *
   * Throwing occurs only if setting the option can fail and if failure occurs.
   *
   * @tparam O cURL option value
   *
   * @param opt cURL option
   * @returns `*this` to allow method chaining
   */
  template <CURLoption O>
  auto& option(const curl_option<O>& opt) & noexcept(curl_option_always_ok<O>)
  {
    set<O>(opt.value());
    return *this;
  }

  /**
   * Set the cURL option on the handle.
   *
   * Throwing occurs only if setting the option can fail and if failure occurs.
   *
   * @tparam O cURL option value
   *
   * @param opt cURL option
   * @returns `std::move(*this)` to allow method chaining
   */
  template <CURLoption O>
  auto&& option(const curl_option<O>& opt) && noexcept(curl_option_always_ok<O>)
  {
    set<O>(opt.value());
    return std::move(*this);
  }

  /**
   * Perform a blocking network transfer with the easy handle.
   *
   * This throws on error and should not be called concurrently.
   *
   * @returns `*this` to allow method chaining
   */
  auto& operator()() &
  {
    auto status = curl_easy_perform(handle_);
    PDXKA_CURL_NOT_OK(status)
      PDXKA_CURL_ERROR_THROW(status, "curl_easy_perform failed");
    return *this;
  }

  /**
   * Perform a blocking network transfer with the easy handle.
   *
   * This throws on error and should not be called concurrently.
   *
   * @returns `std::move(*this)` to allow method chaining
   */
  auto&& operator()() &&
  {
    return std::move((*this)());
  }

private:
  CURL* handle_;

  /**
   * Move from the other handle.
   *
   * The other handle's raw handle is copied and then zeroed.
   */
  void move(curl_handle&& other) noexcept
  {
    handle_ = other.handle_;
    other.handle_ = nullptr;
  }

  /**
   * Clean up the easy handle if it is still being used.
   */
  void destroy_handle() noexcept
  {
    // if handle_ is nullptr nothing is done
    curl_easy_cleanup(handle_);
  }

  /**
   * Set the cURL option on the handle with the given value.
   *
   * Throwing occurs only if setting the option can fail and if failure occurs.
   *
   * @tparam O cURL option value
   *
   * @param value cURL option argument value
   */
  template <CURLoption O>
  void set(curl_option_value_type<O> value) noexcept(curl_option_always_ok<O>)
  {
// MSVC warns that extern "C" function may throw since cURL does not provide a
// noexcept macro for C++ to mark its functions as non-throwing
PDXKA_MSVC_WARNING_PUSH()
PDXKA_MSVC_WARNING_DISABLE(5039)
    auto status = curl_easy_setopt(handle_, O, value);
PDXKA_MSVC_WARNING_POP()
    // exception throw branch only available if option can fail
    if constexpr (!curl_option_always_ok<O>)
      PDXKA_CURL_NOT_OK(status)
        PDXKA_CURL_ERROR_THROW(status, "curl_easy_setopt failed");
  }
};

namespace detail {

/**
 * cURL callback function used to write received data.
 *
 * Returns the number of characters written, where if the returned value is
 * less than `n_items`, cURL will interpret this as an error and halt.
 *
 * @param incoming `char*` buffer of data read by cURL, not `NULL`-terminated
 * @param item_size `std::size_t` size of char items, always 1 (unused)
 * @param n_items `std::size_t` number of chars in buffer to write
 * @param stream `void*` address of a `std::stringstream` to put chars in
 */
inline std::size_t curl_writer(
  char* incoming,
  std::size_t /* item_size */,
  std::size_t n_items,
  void* stream) noexcept
{
  // error if either incoming buffer or stream are NULL
  if (!incoming || !stream)
    return 0;
  // counter/number of items read before exception
  std::size_t n = 0;
  // since incoming is not NULL-terminated, just put each char into stream. C
  // code doesn't know how to handle exceptions, so wrap in try/catch
  try {
    for (; n < n_items; n++)
      static_cast<std::stringstream*>(stream)->put(incoming[n]);
  }
  catch (...) {
    std::cerr << PDXKA_PRETTY_FUNCTION_NAME << ": " <<
      boost::current_exception_diagnostic_information() << std::endl;
    return n;
  }
  return n_items;
}

}  // namespace detail

/**
 * Make a HTTP[S] `GET` request to a URL using libcurl.
 *
 * @tparam Os... cURL option values
 *
 * @param url URL to make HTTP[S] `GET` request to
 * @param options Other libcurl options to set
 */
template <CURLoption... Os>
curl_result curl_get(const std::string& url, const curl_option<Os>&... options)
{
  // cURL status + stream to hold response body
  auto status = CURLE_OK;
  std::stringstream stream;
  // handle error buffer. null-terminated so it is always valid
  // TODO: should a handle maintain its own error buffer?
  char errbuf[CURL_ERROR_SIZE] = "";
  // handle with error buffer, callback writer + write target (should not fail)
  auto handle = curl_handle{}
    .option<CURLOPT_ERRORBUFFER>(errbuf)
    .option<CURLOPT_WRITEFUNCTION>(&detail::curl_writer)
    .option<CURLOPT_WRITEDATA>(&stream);
  // perform possibly-failing operations
  try {
    // set URL to make GET request to (errors if no heap space left)
    handle.option<CURLOPT_URL>(url.c_str());
    // set other options that may fail
    (handle.option(options), ...);
    // perform GET request
    handle();
  }
  // catch cURL error for status update
  catch (const curl_error& err) {
    status = err.status();
  }
  // return result
  return {status, errbuf, request_type::get, stream.str()};
}

}  // namespace pdkxa

#endif  // PDXKA_CURL_HH_
