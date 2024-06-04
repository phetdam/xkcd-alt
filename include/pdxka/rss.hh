/**
 * @file rss.hh
 * @author Derek Huang
 * @brief Parse retrieved XKCD RSS feed XML
 * @copyright MIT License
 */

#ifndef PDXKA_RSS_HH_
#define PDXKA_RSS_HH_

#include <cassert>
#include <cstdint>
#include <sstream>
#include <string>
#include <vector>

#include <boost/property_tree/ptree.hpp>
#include <boost/property_tree/xml_parser.hpp>
#include <curl/curl.h>

#include "pdxka/curl.hh"

namespace pdxka {

/**
 * Return const reference to current XKCD RSS feed URL.
 *
 * @note Construct on first use idiom used for static initialization safety.
 */
inline const auto& rss_url()
{
  static std::string url{"https://xkcd.com/rss.xml"};
  return url;
}

/**
 * Get the latest XKCD RSS XML.
 *
 * @param options `curl_option<T>` additional cURL options to set
 */
template <typename... Ts>
inline curl_result get_rss(curl_option<Ts>... options)
{
  return curl_get(rss_url(), options...);
}

/**
 * Return Boost `ptree` holding the latest XKCD RSS XML.
 *
 * @param xml `const std::string&` raw XKCD RSS XML
 *
 * @throws `boost::property_tree::xml_parser::xml_parser_error` if parse fails
 */
inline auto parse_rss(const std::string& xml)
{
  namespace pt = boost::property_tree;
  // property tree we will use to store RSS tree results in
  pt::ptree tree;
  // create stream from rss_string + parse XML (drop comments)
  std::stringstream stream(xml, std::ios_base::in);
  pt::read_xml(stream, tree, pt::xml_parser::no_comments);
  return tree;
}

/**
 * Class to represent an XKCD RSS XML item.
 *
 * Nearly one-to-one mapping with the `<item>`, although the `<description>`
 * tag has image `src`, `title`, `alt` as `img_src`, `img_title`, `img_alt`.
 */
class rss_item {
public:
  using ptree = boost::property_tree::ptree;

  /**
   * Default constructor.
   *
   * Call `from_tree` on the instance to read from a Boost property tree.
   */
  rss_item() = default;

  /**
   * Constructor initializing class with data from a Boost `ptree`.
   *
   * @param tree `const ptree&` tree containing XKCD RSS item data
   */
  rss_item(const ptree& tree)
  {
    from_tree(tree, this);
  }

  /**
   * Return new `rss_item` from Boost `ptree` containing XKCD RSS item data.
   *
   * @param tree `const ptree&` tree containing XKCD TSS item data
   */
  static rss_item from_tree(const ptree& tree)
  {
    rss_item item;
    from_tree(tree, &item);
    return item;
  }

  /**
   * Populate `rss_item` data from Boost `ptree` containing XKCD RSS item data.
   *
   * Returns `item` reference to allow method chaining.
   *
   * @param tree `const ptree&` tree containing XKCD TSS item data
   * @param item `rss_item&` RSS item to populate
   */
  static rss_item& from_tree(const ptree& tree, rss_item& item)
  {
    from_tree(tree, &item);
    return item;
  }

  /**
   * Populate `rss_item` data from Boost `ptree` containing XKCD RSS item data.
   *
   * Returns `item` pointer to allow method chaining.
   *
   * @param tree `const ptree&` tree containing XKCD TSS item data
   * @param item `rss_item*` pointer to RSS item to populate
   */
  static rss_item* from_tree(const ptree& tree, rss_item* item);

  /**
   * Return the title of the XKCD comic.
   */
  const auto& title() const noexcept { return title_; }

  /**
   * Return the URL link to the XKCD comic.
   */
  const auto& link() const noexcept { return link_; }

  /**
   * Return the URL to the XKCD comic's image source.
   */
  const auto& img_src() const noexcept { return img_src_; }

  /**
   * Return the XKCD comic's image title.
   *
   * @note This is typically the same as the image's alt text.
   */
  const auto& img_title() const noexcept { return img_title_; }

  /**
   * Return the XKCD comic's image alt text.
   */
  const auto& img_alt() const noexcept { return img_alt_; }

  /**
   * Return the XKCD comic's publication date string.
   *
   * Typically the format is something like `Fri, 31 May 2024 04:00:00 -0000`.
   */
  const auto& pub_date() const noexcept { return pub_date_;}

  /**
   * Return the XKCD comic's globally unique identifier.
   *
   * @note This is typically the same as the comic's URL link.
   */
  const auto& guid() const noexcept { return guid_; }

private:
  std::string title_;
  std::string link_;
  std::string img_src_;
  std::string img_title_;
  std::string img_alt_;
  std::string pub_date_;
  std::string guid_;
};

using rss_item_vector = std::vector<rss_item>;

/**
 * Return a `rss_item_vector` from a Boost property tree holding XKCD RSS XML.
 */
rss_item_vector to_item_vector(const boost::property_tree::ptree& rss_tree);

}  // namespace pdxka

#endif  // PDXKA_RSS_HH_
