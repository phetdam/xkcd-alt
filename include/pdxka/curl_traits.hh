/**
 * @file curl_traits.hh
 * @author Derek Huang
 * @brief C++ header for cURL type traits
 * @copyright MIT License
 */

#ifndef PDXKA_CURL_TRAITS_HH_
#define PDXKA_CURL_TRAITS_HH_

#include <cstdint>
#include <type_traits>

#include <curl/curl.h>

namespace pdxka {

/**
 * `CURLOPT_WRITEFUNCTION` callback function pointer type.
 */
using curl_write_callback = std::size_t (*)(
  char* data,
  std::size_t /*always 1*/,
  std::size_t n_elem,
  void* user_data);

/**
 * Traits type that maps a cURL option value to its value type.
 *
 * @tparam opt cURL option value
 */
template <CURLoption opt>
struct curl_option_traits {};

/**
 * Macro to define a new `curl_option_traits` specialization.
 *
 * @param opt cURL option value
 * @param type cURL option value type
 * @param never_fail `true` if `curl_easy_setopt` never fails with this option
 */
#define PDXKA_CURL_OPTION_TRAITS(opt, type, never_fail) \
  template <> \
  struct curl_option_traits<opt> { \
    /* disallow type conversion */ \
    static_assert( \
      std::is_same_v<decltype(opt), CURLoption>, \
      "opt must be CURLoption" \
    ); \
    /* option value type */ \
    using value_type = type; \
    /* option value */ \
    static constexpr CURLoption value = opt; \
    /* never fails */ \
    static constexpr bool always_ok = never_fail; \
  }

/**
 * SFINAE-capable helper for the cURL option value's C type.
 *
 * @tparam opt cURL option value
 */
template <CURLoption opt>
using curl_option_value_type = typename curl_option_traits<opt>::value_type;

/**
 * SFINAE-capable helper for whether setting the cURL option never fails.
 *
 * @tparam opt cURL option value
 */
template <CURLoption opt>
constexpr bool curl_option_always_ok = curl_option_traits<opt>::always_ok;

// note: incomplete
// note: for input string literals/buffers, we use const char* instead
PDXKA_CURL_OPTION_TRAITS(CURLOPT_ERRORBUFFER, char*, true);
PDXKA_CURL_OPTION_TRAITS(CURLOPT_WRITEFUNCTION, curl_write_callback, true);
PDXKA_CURL_OPTION_TRAITS(CURLOPT_WRITEDATA, void*, true);
PDXKA_CURL_OPTION_TRAITS(CURLOPT_URL, const char* /*strengthened*/, false);
PDXKA_CURL_OPTION_TRAITS(CURLOPT_VERBOSE, long, false);
PDXKA_CURL_OPTION_TRAITS(CURLOPT_SSL_VERIFYPEER, long, false);

}  // namespace pdxka

#endif  // PDXKA_CURL_TRAITS_HH_
