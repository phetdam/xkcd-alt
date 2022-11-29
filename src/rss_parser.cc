/**
 * @file rss_parser.cc
 * @author Derek Huang
 * @brief Parse retrieved XKCD RSS feed XML
 * @copyright MIT License
 */

#include <pdxka/rss_parser.h>

#include <cstdlib>
#include <iostream>
#include <sstream>
#include <string>

#include <boost/exception/diagnostic_information.hpp>
#include <boost/property_tree/ptree.hpp>

#include <curl/curl.h>

namespace pdxka {

// current XKCD RSS feed URL
const std::string rss_url = "https://xkcd.com/rss.xml";

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
std::size_t curl_writer(
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
    std::stringstream* sstream = static_cast<std::stringstream*>(stream);
    for (; n < n_items; n++) sstream->put(incoming[n]);
  }
  catch (...) {
    std::cerr << "curl_writer: " <<
      boost::current_exception_diagnostic_information() << std::endl;
    return n;
  }
  return n_items;
}

}  // namespace detail

/**
 * Get the latest XKCD RSS XML.
 *
 * @param url `const std::string&` URL used for XKCD RSS, i.e. `rss_url`
 * @param options `curl_options` cURL options struct
 */
curl_result get_rss(const std::string& url, curl_options options)
{
  // cURL session handle and global error status
  CURL *handle;
  CURLcode status;
  // reason the cURL request has errored out + stream to hold response body
  std::string reason;
  std::stringstream stream;
  // global cURL session init
  status = curl_global_init(CURL_GLOBAL_DEFAULT);
  PDXKA_CURL_ERR_HANDLER(status, reason, "Global init error", clean_global);
  // if good, init the cURL "easy" session
  handle = curl_easy_init();
  // on error, clean up and exit
  if (!handle) {
    reason = "curl_easy_init errored";
    goto clean_easy;
  }
  // set cURL error buffer
  char errbuf[CURL_ERROR_SIZE];
  status = curl_easy_setopt(handle, CURLOPT_ERRORBUFFER, &errbuf);
  PDXKA_CURL_ERR_HANDLER(status, reason, errbuf, clean_easy);
  // set URL to make GET request to
  status = curl_easy_setopt(handle, CURLOPT_URL, url.c_str());
  PDXKA_CURL_ERR_HANDLER(status, reason, errbuf, clean_easy);
  // set cURL callback writer function
  status = curl_easy_setopt(handle, CURLOPT_WRITEFUNCTION, detail::curl_writer);
  PDXKA_CURL_ERR_HANDLER(status, reason, errbuf, clean_easy);
  // set cURL callback writer function's write target
  status = curl_easy_setopt(handle, CURLOPT_WRITEDATA, &stream);
  PDXKA_CURL_ERR_HANDLER(status, reason, errbuf, clean_easy);
  // set cURL options
  if (options.verbose) {
    status = curl_easy_setopt(handle, CURLOPT_VERBOSE, 1L);
    PDXKA_CURL_ERR_HANDLER(status, reason, errbuf, clean_easy);
  }
  if (!options.no_verify_peer) {
    status = curl_easy_setopt(handle, CURLOPT_SSL_VERIFYPEER, 1L);
    PDXKA_CURL_ERR_HANDLER(status, reason, errbuf, clean_easy);
  }
  if (!options.no_verify_host) {
    status = curl_easy_setopt(handle, CURLOPT_SSL_VERIFYHOST, 1L);
    PDXKA_CURL_ERR_HANDLER(status, reason, errbuf, clean_easy);
  }
  // perform GET request
  status = curl_easy_perform(handle);
  PDXKA_CURL_ERR_HANDLER(status, reason, errbuf, clean_easy);
  // clean up both "easy" and global sessions
clean_easy:
  curl_easy_cleanup(handle);
clean_global:
  curl_global_cleanup();
  return {status, reason, request_type::GET, stream.str()};
}

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
