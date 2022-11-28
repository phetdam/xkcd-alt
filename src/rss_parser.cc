/**
 * @file rss_parser.cc
 * @author Derek Huang
 * @brief Parse retrieved XKCD RSS feed XML
 * @copyright MIT License
 */

#include <pdxka/rss_parser.h>

#include <sstream>
#include <string>

#ifndef PDXKA_USE_CURL
#include <boost/asio/connect.hpp>
#include <boost/asio/ip/tcp.hpp>
#include <boost/beast/core.hpp>
#include <boost/beast/http.hpp>
#include <boost/beast/version.hpp>
#endif  // PDXKA_USE_CURL
#include <boost/property_tree/ptree.hpp>
#ifdef PDXKA_USE_CURL
#include <curl/curl.h>
#endif  // PDXKA_USE_CURL

namespace pdxka {

// XKCD RSS feed URL
const std::string xkcd_rss_url = "https://xkcd.com/rss.xml";

// default HTTP port
const unsigned int default_port = 8070;

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
  item->title_ = tree.get<std::string>("item.title");
  item->link_ = tree.get<std::string>("item.link");
  item->pub_date_ = tree.get<std::string>("item.pubDate");
  item->guid_ = tree.get<std::string>("item.guid");
  // img data is all from <description> tag, but we can create a property tree
  // from the img data XML text to get src, title, alt conveniently
  const auto desc_tree{parse_rss(tree.get<std::string>("item.description"))};
  item->img_src_ = desc_tree.get<std::string>("img.xmlattr.src");
  item->img_title_ = desc_tree.get<std::string>("img.xmlattr.title");
  item->img_alt_ = desc_tree.get<std::string>("img.xmlattr.alt");
  return item;
}

/**
 * Get the latest XKCD RSS XML and return it as a `std::string`.
 *
 * Implementation uses Boost.Beast by default unless `PDXKA_USE_CURL` was
 * defined during compilation, in which case cURL is used instead.
 *
 * Throws an exception if any errors are encountered.
 *
 * @param rss_url `const std::string&` URL used for XKCD RSS. If not specified,
 *    the default URL used is given by `xkcd_rss_url`.
 */
std::string get_rss(const std::string& rss_url, unsigned int port)
{
#ifdef PDXKA_USE_CURL
#error "pdxka does not yet support using cURL, please use Boost.Beast instead"
#else
  // the following is closely copied from the Boost.Beast simple HTTP client
  // example, as it is not well understood by me. one can argue that using
  // Boost.Beast for a HTTP GET request is analagous to using a sledgehammer to
  // drive in a nail; it is better used by library writers.
  namespace beast = boost::beast;     // from <boost/beast.hpp>
  namespace http = beast::http;       // from <boost/beast/http.hpp>
  namespace net = boost::asio;        // from <boost/asio.hpp>
  using tcp = net::ip::tcp;           // from <boost/asio/ip/tcp.hpp>
  // io_context is required for all I/O
  net::io_context io_context;
  // I/O objects: first resolves URLs, second is for data stream
  tcp::resolver resolver(io_context);
  beast::tcp_stream stream(io_context);
  // look up the XKCD RSS URL
  const auto urls{resolver.resolve(rss_url, std::to_string(port).c_str())};
  // make the connection on the IP address we get from a lookup
  stream.connect(urls);
  // set up HTTP GET request message from URL root (using HTTP 1.1 protocol)
  http::request<http::string_body> req{http::verb::get, "/", 11};
  req.set(http::field::host, rss_url);
  req.set(http::field::user_agent, BOOST_BEAST_VERSION_STRING);
  // send the HTTP request to the remote host
  http::write(stream, req);
  // container to hold the response + required reading buffer
  http::response<http::dynamic_body> response;
  beast::flat_buffer buffer;
  // receive the HTTP response and write the buffer data to a string
  http::read(stream, buffer, response);
  std::string rss = beast::buffers_to_string(buffer.data());
  // gracefully close the socket
  beast::error_code errcode;
  stream.socket().shutdown(tcp::socket::shutdown_both, errcode);
  // sometimes errcode is not_connected, so ignore it. return if no error
  if (errcode && errcode != beast::errc::not_connected)
    throw beast::system_error(errcode);
  return rss;
#endif  // PDXKA_USE_CURL
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
