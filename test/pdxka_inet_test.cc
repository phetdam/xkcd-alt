/**
 * @file pdxka_inet_test.cc
 * @author Derek Huang
 * @brief C++ program to check if Intener connection is available
 * @copyright MIT License
 *
 * @file
 *
 * This program is used with the pdxka_live_test.cmake CMake script as a
 * pre-test step to skip launching any tests of xkcd-alt that will make network
 * connection if there is "no Internet". Therefore, when appropriate, these
 * tests are marked as skipped without the delay of retry/timeout loops.
 */

#if defined(_WIN32)
#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif  // WIN32_LEAN_AND_MEAN
#include <Windows.h>
#include <combaseapi.h>
#include <objbase.h>
#include <netlistmgr.h>
#else
#include <ifaddrs.h>
#include <net/if.h>  // for IFF_UP
#include <sys/socket.h>
#include <sys/types.h>
#endif  // !defined(_WIN32)

#include <cstdlib>
#include <cstring>
#include <ios>
#include <iostream>
#include <stdexcept>
#include <string>
#include <utility>

#ifdef _WIN32
#include "pdxka/com.hh"
#endif  // _WIN32

namespace {

#if defined(_WIN32)
/**
 * Indicate whether or not the local machine is connected to Internet.
 */
bool has_inet_connection(INetworkListManager& mgr)
{
  // get status and value
  VARIANT_BOOL res;
  auto hres = mgr.get_IsConnectedToInternet(&res);
  if (FAILED(hres))
    throw pdxka::com_error{hres, "get_IsConnectedToInternet failed"};
  // convert to actual bool
  return res == VARIANT_TRUE;
}
#else
/**
 * Exception type to indicate a POSIX function has failed and set `errno`.
 */
class posix_error : public std::runtime_error {
public:
  /**
   * Default ctor.
   *
   * The current value of `errno` is used to construct the exception message.
   */
  posix_error() : posix_error{errno} {}

  /**
   * Ctor.
   *
   * @param err `errno` status
   */
  posix_error(int err)
    : std::runtime_error{"Error: " + std::string{std::strerror(err)}}
  {}

  /**
   * Ctor.
   *
   * @param err `errno` status
   * @param message Message
   */
  posix_error(int err, const std::string& message)
    : std::runtime_error{"Error: " + message + ": " + std::strerror(err)}
  {}

  /**
   * Return the `errno` value used in the exception.
   */
  auto err() const noexcept { return err_; }

private:
  int err_;
};

/**
 * Manager of `ifaddrs` network interfaces linked list.
 */
class netifaddrs_list {
public:
  /**
   * Iterator for the `netifaddrs_list`.
   *
   * This models *LegacyForwardIterator* and the C++20 `forward_iterator`. It
   * is intended to allow iteration through the `netifaddrs_list` like follows:
   *
   * @code{.cc}
   * netifaddrs_list netifs;
   * for (const auto& netif : netifs)
   *   std::cout << "Network interface: " << netif.ifa_name << std::endl;
   * @endcode
   *
   * This iterator is trivially copyable and copy-assignable.
   */
  class iterator {
  public:
    /**
     * Default ctor.
     *
     * This constructs the end iterator with `nullptr` data.
     */
    iterator() noexcept : iterator{nullptr} {}

    /**
     * Ctor.
     *
     * Constructs from an `ifaddrs*`, with `nullptr` for the end iterator.
     */
    iterator(const ifaddrs* data) noexcept : data_{data} {}

    /**
     * Obtain a reference to the current value.
     *
     * Behavior is undefined if the iterator is an end iterator.
     */
    const auto& operator*() const noexcept
    {
      return *data_;
    }

    /**
     * Check for equality against another iterator.
     */
    bool operator==(iterator other) const noexcept
    {
      return data_ == other.data_;
    }

    /**
     * Check for inequality against another iterator.
     *
     * @note In C++20 this is synthesized from `operator==`.
     */
    bool operator!=(iterator other) const noexcept
    {
      return !(*this == other);
    }

    /**
     * Advance position by one (pre-increment).
     *
     * Behavior is undefined if the iterator is an end iterator.
     */
    auto& operator++() noexcept
    {
      data_ = data_->ifa_next;
      return *this;
    }

    /**
     * Advance position by one (post-increment).
     *
     * Behavior is undefined if the iterator is an end iterator.
     *
     * @note This returns a value to satisfy `std::incrementable`.
     */
    auto operator++(int) noexcept
    {
      auto res = *this;
      ++*this;
      return res;
    }

    /**
     * Provide access to the `1faddrs` members.
     *
     * @note This is provided to satisfy *LegacyInputIterator*.
     */
    auto operator->() const noexcept
    {
      return data_;
    }

  private:
    const ifaddrs* data_;
  };

  /**
   * Ctor.
   *
   * This calls `getifaddrs` to retrieve the network interfaces linked list.
   */
  netifaddrs_list()
  {
    if (getifaddrs(&head_))
      throw posix_error{};
  }

  /**
   * Deleted copy ctor.
   */
  netifaddrs_list(const netifaddrs_list&) = delete;

  /**
   * Move ctor.
   */
  netifaddrs_list(netifaddrs_list&& other) noexcept
  {
    move(std::move(other));
  }

  /**
   * Move assignment operator.
   */
  auto& operator=(netifaddrs_list& other) noexcept
  {
    free();
    move(std::move(other));
    return *this;
  }

  /**
   * Dtor.
   */
  ~netifaddrs_list()
  {
    free();
  }

  /**
   * Return the head of the list.
   */
  auto head() const noexcept { return head_; }

  /**
   * Return iterator to the first `ifaddrs`.
   */
  iterator begin() const noexcept
  {
    return head_;
  }

  /**
   * Return iterator to one past the last `ifaddrs`.
   */
  iterator end() const noexcept
  {
    return nullptr;
  }

  /**
   * Implicit conversion to `ifaddrs*`.
   *
   * Can also be used to indicate if data is present.
   */
  operator ifaddrs*() const noexcept
  {
    return head_;
  }

private:
  ifaddrs* head_;

  /**
   * Move from another `netifaddrs_list`.
   *
   * After completion the moved-from `netifaddrs_list` will have `nullptr` data.
   */
  void move(netifaddrs_list&& other) noexcept
  {
    head_ = other.head_;
    other.head_ = nullptr;
  }

  /**
   * Free the `ifaddrs` list if one is owned.
   */
  void free() noexcept
  {
    if (head_)
      freeifaddrs(head_);
  }
};

/**
 * Check if the local machine is connected to the Internet.
 *
 * That is, we check if there is at least one non-loopback network interface
 * that has an IPv4 or IPv6 address for Internet connection, i.e. `AF_INET` or
 * `AF_INET6`, that is actively running, ie. with `IFF_UP`.
 *
 * @param netifs List of network interface addresses
 */
bool has_inet(const netifaddrs_list& netifs) noexcept
{
  for (const auto& nif : netifs) {
    // address family/socket domain + network device flags
    auto family = nif.ifa_addr->sa_family;
    auto flags = nif.ifa_flags;
    // if criteria matches, done
    if (
      (flags & IFF_UP) &&                        // interface running
      !(flags & IFF_LOOPBACK) &&                 // not a loopback address
      (family == AF_INET || family == AF_INET6)  // IPv4 or IPv6
    )
      return true;
  }
  return false;
}
#endif  // !defined(_WIN32)

}  // namespace

int main()
{
#if defined(_WIN32)
  // single-threaded COM initialization
  pdxka::coinit_context com{COINIT_APARTMENTTHREADED};
  // create network list manager instance
  pdxka::com_ptr<INetworkListManager> mgr;
  // check if connected + print message
  auto have_inet = has_inet_connection(*mgr);
#else
  // get list of network interfaces
  netifaddrs_list netifs;
  // check if connected + print message
  auto have_inet = has_inet(netifs);
#endif  // !defined(_WIN32)
  std::cout << "Internet: " << (have_inet ? "Yes" : "No") << std::endl;
  return have_inet ? EXIT_SUCCESS : EXIT_FAILURE;
}
