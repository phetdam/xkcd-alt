/**
 * @file rss.cc
 * @author Derek Huang
 * @brief Parse retrieved XKCD RSS feed XML
 * @copyright MIT License
 */

#include "pdxka/rss.hh"

#include <cstdlib>
#include <iostream>
#include <sstream>
#include <string>

#include <boost/exception/diagnostic_information.hpp>
#include <boost/property_tree/ptree.hpp>
#include <curl/curl.h>

#include "pdxka/common.h"

namespace pdxka {

/**
 * Populate `rss_item` data from Boost `ptree` containing XKCD RSS item data.
 *
 * Returns `item` pointer to allow method chaining.
 *
 * @param tree `const ptree&` tree containing XKCD TSS item data
 * @param item `rss_item*` pointer to RSS item to populate
 */
rss_item* rss_item::from_tree(
  const boost::property_tree::ptree& tree, rss_item* item)
{
  assert(item);
  // non-img data are directly accessible (can throw)
  item->title_ = tree.get<std::string>("title");
  item->link_ = tree.get<std::string>("link");
  item->pub_date_ = tree.get<std::string>("pubDate");
  item->guid_ = tree.get<std::string>("guid");
  // img data is all from <description> tag, but we can create a property tree
  // from the img data XML text to get src, title, alt conveniently
  const auto desc_tree{parse_rss(tree.get<std::string>("description"))};
  item->img_src_ = desc_tree.get<std::string>("img.<xmlattr>.src");
  item->img_title_ = desc_tree.get<std::string>("img.<xmlattr>.title");
  item->img_alt_ = desc_tree.get<std::string>("img.<xmlattr>.alt");
  return item;
}

/**
 * Return a `rss_item_vector` from a Boost property tree holding XKCD RSS XML.
 */
rss_item_vector to_item_vector(const boost::property_tree::ptree& rss_tree)
{
  // RSS <item> groups subtree under rss.channel (can throw)
  const auto& rss_items_tree = rss_tree.get_child("rss.channel");
  // RSS item vector + populate with only the <item> items
  rss_item_vector rss_items;
  for (const auto& item : rss_items_tree) {
    if (item.first == "item")
      rss_items.emplace_back(item.second);
  }
  return rss_items;
}

}  // namespace pdxka
