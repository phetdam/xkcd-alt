/**
 * @file netif.hh
 * @author Derek Huang
 * @brief C++ header for network interface helpers
 * @copyright MIT License
 */

#ifndef PDXKA_NETIF_HH_
#define PDXKA_NETIF_HH_

#include <ifaddrs.h>
#include <net/if.h>  // for IFF_UP

#include <cstdint>
#include <iterator>
#include <utility>

#include "pdxka/posix.hh"

namespace pdxka {

/**
 * `ifaddrs` network interfaces linked list manager with unique ownership.
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
    // iterator traits
    using difference_type = std::ptrdiff_t;
    using value_type = ifaddrs;
    using pointer = const ifaddrs*;
    using reference = const ifaddrs&;
    using iterator_category = std::forward_iterator_tag;

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
     * Provide access to the `ifaddrs` members.
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
  auto& operator=(netifaddrs_list&& other) noexcept
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
    return {};
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

}  // namespace pdxka

#endif  // PDXKA_NETIF_HH_
