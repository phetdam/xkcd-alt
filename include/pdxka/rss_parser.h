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

namespace pdxka {

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

extern const std::string xkcd_rss_url;
extern const unsigned int default_port;

/**
 * Return Boost `ptree` holding the latest XKCD RSS XML.
 *
 * @param rss_string `const std::string&` raw XKCD RSS XML
 *
 * @throws `boost::property_tree::xml_parser::xml_parser_error` if parse fails
 */
inline boost::property_tree::ptree parse_rss(const std::string& rss_string)
{
  namespace pt = boost::property_tree;
  // property tree we will use to store RSS tree results in
  pt::ptree rss_tree;
  // create stream from rss_string + parse XML (drop comments)
  std::stringstream rss_stream(rss_string, std::ios_base::in);
  pt::read_xml(rss_stream, rss_tree, pt::xml_parser::no_comments);
  return rss_tree;
}

std::string get_rss(const std::string& rss_url, unsigned int port);

inline std::string get_rss(const std::string& rss_url)
{
  return get_rss(rss_url, default_port);
}

inline std::string get_rss() { return get_rss(xkcd_rss_url); }

rss_item_vector to_item_vector(const boost::property_tree::ptree& rss_tree);

}  // namespace pdxka

#endif  // PDXKA_RSS_PARSER_H_
