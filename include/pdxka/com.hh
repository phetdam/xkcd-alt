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
#include <ComSvcs.h>
#include <oaidl.h>    // IDispatch
#include <objbase.h>
#include <netlistmgr.h>
#include <Unknwn.h>   // IUnknown
#include <wincodec.h>

#include <ios>
#include <sstream>
#include <stdexcept>
#include <string>
#include <type_traits>
#include <utility>

#ifndef PDXKA_COM_HH_
#define PDXKA_COM_HH_

namespace pdxka {

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
  com_error(HRESULT err) : com_error{err, "COM error"} {}

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
    auto hres = CoInitializeEx(nullptr, opts);
    if (FAILED(hres))
      throw com_error{hres, "COM initialization failed"};
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

/**
 * COM object traits type.
 *
 * This is used to determine the `CLSID` and `IID` of the COM object. A valid
 * specialization should contain the `clsid` and `iid` static constexpr members
 * which are simply const references to the `CLSID` and `IID`.
 *
 * @tparam T `IUnknown` subclass
 */
template <typename T>
struct com_traits {};

/**
 * Specialization for the `INetworkListManager`.
 */
template <>
struct com_traits<INetworkListManager> {
  static constexpr const auto& clsid = CLSID_NetworkListManager;
  static constexpr const auto& iid = IID_INetworkListManager;
};

/**
 * Specialization for the `IWICImagingFactory`.
 *
 * To use this specialization you must link against `WindowsCodec.lib`.
 */
template <>
struct com_traits<IWICImagingFactory> {
  static constexpr const auto& clsid = CLSID_WICImagingFactory;
  static constexpr const auto& iid = IID_IWICImagingFactory;
};

/**
 * Specialization for the `ITransactionContext`.
 */
template <>
struct com_traits<ITransactionContext> {
  static constexpr const auto& clsid = CLSID_TransactionContext;
  static constexpr const auto& iid = IID_ITransactionContext;
};

/**
 * COM object pointer class.
 *
 * This provides a natural shared pointer-like functionality using the existing
 * reference-counting functionality of COM objects.
 *
 * @tparam T COM interface class type, e.g. `INetworkListManager`
 */
template <typename T>
class com_ptr {
public:
  // requires that the com_traits<T> specialization members exist
  static_assert(is_com_unknown_v<T>, "must be a COM interface");
  using value_type = T;

  /**
   * Default ctor.
   *
   * Create a COM object on the local system with no parent for all contexts.
   */
  com_ptr() : com_ptr{nullptr, CLSCTX_ALL} {}

  /**
   * Ctor.
   *
   * Create a COM object on the local system with the given parent and context.
   *
   * @note This uses `CoCreateInstance` so is less efficient for bulk creation.
   *
   * @param outer The parent `IUnknown` or `nullptr` if not part of aggregate
   * @param ctx Execution context to run the COM object in, e.g. `CLSCTX_ALL`
   */
  com_ptr(LPUNKNOWN outer, DWORD ctx)
  {
    // CLSID and IID to use
    const auto& clsid = com_traits<T>::clsid;
    const auto& iid = com_traits<T>::iid;
    // create object
    LPVOID obj;
    auto hres = CoCreateInstance(clsid, outer, ctx, iid, &obj);
    if (FAILED(hres))
      throw com_error{hres, "Failed to create COM object instance"};
    // on success, set pointer
    ptr_ = static_cast<T*>(obj);
  }

  /**
   * Copy ctor.
   *
   * This increments the reference count of the underlying COM object.
   */
  com_ptr(const com_ptr& other) noexcept
  {
    from(other);
  }

  /**
   * Move ctor.
   *
   * This transfers ownership without incrementing the reference count.
   */
  com_ptr(com_ptr&& other) noexcept
  {
    from(std::move(other));
  }

  /**
   * Copy assignment operator.
   */
  auto& operator=(const com_ptr& other) noexcept
  {
    destroy();
    from(other);
    return *this;
  }

  /**
   * Move asignment operator.
   */
  auto& operator=(com_ptr&& other) noexcept
  {
    destroy();
    from(std::move(other));
    return *this;
  }

  /**
   * Dtor.
   *
   * Decrements the reference count on the COM object if one is owned.
   */
  ~com_ptr()
  {
    destroy();
  }

  /**
   * Return the raw COM interface pointer.
   */
  auto ptr() const noexcept { return ptr_; }

  /**
   * Access an underlying member of the COM object pointer.
   *
   * @warning Calling `AddRef` or `Release` using `operator->` introduces the
   *  risk of reference leaking and double delete respectively.
   */
  auto operator->() const noexcept
  {
    return ptr_;
  }

  /**
   * Implicitly convert to the raw COM interface pointer.
   */
  operator T*() const noexcept
  {
    return ptr_;
  }

private:
  T* ptr_{};

  /**
   * Implement copy from another `com_ptr`.
   *
   * The COM object pointer is copied and its reference count incremented.
   */
  void from(const com_ptr& other) noexcept
  {
    ptr_ = other.ptr_;
    ptr_->AddRef();
  }

  /**
   * Implement move from another `com_ptr`.
   *
   * The COM object pointer is copied and then set to `nullptr` for `other`.
   */
  void from(com_ptr&& other) noexcept
  {
    ptr_ = other.ptr_;
    other.ptr_ = nullptr;
  }

  /**
   * Decrements the reference count of the COM object is one is owned.
   */
  void destroy() noexcept
  {
    if (ptr_)
      ptr_->Release();
  }
};

}  // namespace pdxka

#endif  // PDXKA_COM_HH_
