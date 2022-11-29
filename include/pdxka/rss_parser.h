/**
 * @file rss_parser.h
 * @author Derek Huang
 * @brief Parse retrieved XKCD RSS feed XML
 * @copyright MIT License
 */

#ifndef PDXKA_RSS_PARSER_H_
#define PDXKA_RSS_PARSER_H_

#include <cassert>
#include <cstdint>
#include <sstream>
#include <string>
#include <vector>

#include <boost/property_tree/ptree.hpp>
#include <boost/property_tree/xml_parser.hpp>

#include <curl/curl.h>

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
  while(0)

namespace pdxka {

extern const std::string rss_url;

/**
 * Immutable POD class holding options for get_rss.
 *
 * @param verbose `bool` to make cURL operate verbosely if `true`
 * @param no_verify_peer `bool` to make cURL not verify server's cert if `true`
 * @param no_verify_host `bool` to make cURL not verify server's id if `true`
 */
struct curl_options {
  const bool verbose;
  const bool no_verify_peer;
  const bool no_verify_host;
};

/**
 * Enum class for the `curl_result` HTTP[S] request type.
 */
enum class request_type {
  GET,
  POST
};

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
 * Return Boost `ptree` holding the latest XKCD RSS XML.
 *
 * @param xml `const std::string&` raw XKCD RSS XML
 *
 * @throws `boost::property_tree::xml_parser::xml_parser_error` if parse fails
 */
inline boost::property_tree::ptree parse_rss(const std::string& xml)
{
  namespace pt = boost::property_tree;
  // property tree we will use to store RSS tree results in
  pt::ptree tree;
  // create stream from rss_string + parse XML (drop comments)
  std::stringstream stream(xml, std::ios_base::in);
  pt::read_xml(stream, tree, pt::xml_parser::no_comments);
  return tree;
}

namespace detail {

std::size_t curl_writer(
  char* incoming,
  std::size_t /* item_size */,
  std::size_t n_items,
  void* stream) noexcept;

}  // namespace detail

curl_result get_rss(const std::string& url, curl_options options = {});

/**
 * Get the latest XKCD RSS XML.
 *
 * Uses `rss_url` as the URL to the XKCD RSS XML.
 *
 * @param options `curl_options` cURL options struct
 */
inline curl_result get_rss(curl_options options = {})
{
  return get_rss(rss_url, options);
}

/**
 * Class to represent an XKCD RSS XML item.
 *
 * Nearly one-to-one mapping with the `<item>`, although the `<description>`
 * tag has image `src`, `title`, `alt` as `img_src`, `img_title`, `img_alt`.
 */
class rss_item {
  using ptree = boost::property_tree::ptree;
public:
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
  rss_item(const ptree& tree) { from_tree(tree, this); }

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

  static rss_item* from_tree(const ptree& tree, rss_item* item);

  const std::string& title() const { return title_; }
  const std::string& link() const { return link_; }
  const std::string& img_src() const { return img_src_; }
  const std::string& img_title() const { return img_title_; }
  const std::string& img_alt() const { return img_alt_; }
  const std::string& pub_date() const { return pub_date_;}
  const std::string& guid() const { return guid_; }

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

rss_item_vector to_item_vector(const boost::property_tree::ptree& rss_tree);

}  // namespace pdxka

#endif  // PDXKA_RSS_PARSER_H_
