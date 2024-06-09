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

#include <boost/exception/diagnostic_information.hpp>
#include <curl/curl.h>

#include "pdxka/common.h"

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
        throw std::runtime_error{
          PDXKA_PRETTY_FUNCTION_NAME +
          std::string{": libcurl initialization failed: "} +
          curl_easy_strerror(status)
        };
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
    if (!(handle_ = curl_easy_init()))
      throw std::runtime_error{
        PDXKA_PRETTY_FUNCTION_NAME + std::string{": curl_easy_init errored"}
      };
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
  curl_handle(curl_handle&& other) noexcept : handle_{other.handle_}
  {
    other.handle_ = nullptr;
  }

  /**
   * Move assignment operator.
   *
   * @param other Handle to transfer ownership from
   */
  auto& operator=(curl_handle&& other) noexcept
  {
    handle_ = other.handle_;
    other.handle_ = nullptr;
    return *this;
  }

  /**
   * Dtor.
   *
   * Clean up the easy handle if it is still owned.
   */
  ~curl_handle()
  {
    // if handle_ is nullptr nothing is done
    curl_easy_cleanup(handle_);
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

private:
  CURL* handle_;
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
    auto sstream = static_cast<std::stringstream*>(stream);
    for (; n < n_items; n++) sstream->put(incoming[n]);
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
 * @param url URL to make HTTP[S] `GET` request to
 * @param options `curl_option<T>` additional libcurl options to set
 */
template <typename... Ts>
curl_result curl_get(const std::string& url, const curl_option<Ts>&... options)
{
  // cURL session handle and global error status
  curl_handle handle;
  CURLcode status;
  // reason the cURL request has errored out + stream to hold response body
  std::string reason;
  std::stringstream stream;
  // set cURL error buffer, callback writer function, and the write target
  char errbuf[CURL_ERROR_SIZE];
  curl_easy_setopt(handle, CURLOPT_ERRORBUFFER, &errbuf);
  curl_easy_setopt(handle, CURLOPT_WRITEFUNCTION, detail::curl_writer);
  curl_easy_setopt(handle, CURLOPT_WRITEDATA, &stream);
  // set URL to make GET request to (errors if no heap space left)
  status = curl_easy_setopt(handle, CURLOPT_URL, url.c_str());
  PDXKA_CURL_ERR_HANDLER(status, reason, errbuf, done);
  // set cURL options (only if exit status is good) using fold
  (
    [&status, &handle, &options]
    {
      PDXKA_CURL_OK(status)
        status = curl_easy_setopt(handle, options.name(), options.value());
    }(),
    ...
  );
  // check last error and clean up if necessary
  PDXKA_CURL_ERR_HANDLER(status, reason, errbuf, done);
  // perform GET request
  status = curl_easy_perform(handle);
  PDXKA_CURL_ERR_HANDLER(status, reason, errbuf, done);
done:
  return {status, reason, request_type::get, stream.str()};
}

}  // namespace pdkxa

#endif  // PDXKA_CURL_HH_
