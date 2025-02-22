/**
 * @file com.hh
 * @author Derek Huang
 * @brief C++ header for COM automation helpers
 * @copyright MIT License
 */

// reduce Windows.h include size
// TODO: to be nice to downstream consumers we might not want to define this.
// however, we can propagate it as a usage requirement with a CMake target
#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif  // WIN32_LEAN_AND_MEAN

#include <Windows.h>
#include <combaseapi.h>
#include <errhandlingapi.h>
#include <netlistmgr.h>
#include <objbase.h>

#include <ios>
#include <sstream>
#include <stdexcept>
#include <string>
#include <type_traits>

#ifndef PDXKA_COM_HH_
#define PDXKA_COM_HH_

namespace pdxka {

/**
 * Traits class to indicate a type is a COM interface inheriting `IUnknown`.
 *
 * @tparam T type
 */
template <typename T>
struct is_com_unknown : std::bool_constant<std::is_base_of_v<IUnknown, T>> {};

/**
 * Indicate that a type is a COM interface inheriting `IUnknown`.
 *
 * @tparam T type
 */
template <typename T>
constexpr bool is_com_unknown_v = is_com_unknown<T>::value;

/**
 * Traits class to indicate a type is a COM interface inheriting `IDispatch`.
 *
 * @tparam T type
 */
template <typename T>
struct is_com_dispatch : std::bool_constant<std::is_base_of_v<IDispatch, T>> {};

/**
 * Indicate that a type is a COM interface inheriting `IDispatch`.
 *
 * @tparam T type
 */
template <typename T>
constexpr bool is_com_dispatch_v = is_com_dispatch<T>::value;

namespace detail {

/**
 * Convert an integral value into a hexadecimal string.
 *
 * @note Should conider making this more broadly available later.
 *
 * @tparam T Integral type
 *
 * @param v Value to convert
 */
template <typename T, typename = std::enable_if_t<std::is_integral_v<T>>>
auto to_hex_string(T v)
{
  std::stringstream ss;
  ss << std::hex << v;
  return ss.str();
}

}  // namespace detail

/**
 * An exception representing a COM error.
 */
class com_error : public std::runtime_error {
public:
  /**
   * Ctor.
   *
   * @param err `HRESULT` status
   * @param text User-defined message text
   */
  com_error(HRESULT err, const std::string& text)
    : std::runtime_error{
        "Error: " + text + ". HRESULT: " + detail::to_hex_string(err)
      },
      err_{err}
  {}

  /**
   * Ctor.
   *
   * This ctor uses `HRESULT_FROM_WIN32(GetLastError())` to retrieve the error.
   *
   * @param text User-defined message text
   */
  com_error(const std::string& text)
    : com_error{HRESULT_FROM_WIN32(GetLastError()), text}
  {}

  /**
   * Return the `HRESULT` error code.
   */
  auto error() const noexcept { return err_; }

private:
  HRESULT err_;
};

/**
 * COM initialization context.
 *
 * Creates and manages the COM context for the current thread in a scope.
 */
class coinit_context {
public:
  /**
   * Ctor.
   *
   * Initialize the COM library for use by the calling thread.
   *
   * @param opts COM initialization options
   */
  coinit_context(DWORD opts = COINIT_MULTITHREADED)
  {
    // TODO: consider using a "nicer" construct:
    //
    // CoInitializeEx(nullptr, opts) |
    //   except | "COM initialization failed";
    //
    // this can be generalized to a callable off the HRESULT, e.g.
    //
    // CoInitializeEx(nullptr, opts) |
    //   except | [](auto) { throw com_error{"COM initialization failed"}; };
    //
    if (FAILED(CoInitializeEx(nullptr, opts)))
      throw com_error{"COM initialization failed"};
  }

  /**
   * Dtor.
   *
   * Uninitializes COM for the given thread.
   */
  ~coinit_context()
  {
    CoUninitialize();
  }
};

}  // namespace pdxka

#endif  // PDXKA_COM_HH_
