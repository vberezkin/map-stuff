//#include <boost/beast.hpp>

#include <fstream>

#include <boost/asio/connect.hpp>
#include <boost/asio/ip/tcp.hpp>
#include <boost/asio/ssl/error.hpp>
#include <boost/asio/ssl/stream.hpp>
#include <boost/beast/core.hpp>
#include <boost/beast/http.hpp>
#include <boost/beast/ssl.hpp>
#include <boost/beast/version.hpp>

#include "downloader.h"

namespace beast = boost::beast;  // from <boost/beast.hpp>
namespace http = beast::http;    // from <boost/beast/http.hpp>
namespace net = boost::asio;     // from <boost/asio.hpp>
namespace ssl = net::ssl;        // from <boost/asio/ssl.hpp>
using tcp = net::ip::tcp;        // from <boost/asio/ip/tcp.hpp>

namespace {
auto schemeToPort(Scheme s) {
  switch (s) {
    case Scheme::http:
      return 80;
    case Scheme::https:
      return 443;
  }
}

//  TODO
void httpDownload(const Uri& uri, const std::filesystem::path& path) {
  auto version = 11;

  // The io_context is required for all I/O
  net::io_context ioc;

  // These objects perform our I/O
  tcp::resolver resolver(ioc);
  beast::tcp_stream stream(ioc);

  // Look up the domain name
  auto const results =
      resolver.resolve(uri.host, std::to_string(schemeToPort(uri.scheme)));

  // Make the connection on the IP address we get from a lookup
  stream.connect(results);

  // Set up an HTTP GET request message
  http::request<http::string_body> req{http::verb::get, uri.target, version};
  req.set(http::field::host, uri.host);
  req.set(http::field::user_agent, BOOST_BEAST_VERSION_STRING);

  // Send the HTTP request to the remote host
  http::write(stream, req);

  // This buffer is used for reading and must be persisted
  beast::flat_buffer buffer;

  // Declare a container to hold the response
  // http::response<http::dynamic_body> res;
  http::response<http::vector_body<char>> res;

  // Receive the HTTP response
  http::read(stream, buffer, res);

  // Write the message to file
  {
    std::ofstream file{path, std::ios::binary | std::ios::out};
    const auto& data = res.body();
    std::copy(data.begin(), data.end(), std::ostreambuf_iterator(file));
  }

  // Gracefully close the socket
  beast::error_code ec;
  stream.socket().shutdown(tcp::socket::shutdown_both, ec);

  // not_connected happens sometimes
  // so don't bother reporting it.
  //
  if (ec && ec != beast::errc::not_connected)
    throw beast::system_error{ec};

  // If we get here then the connection is closed gracefully
}

void httpsDownload(const Uri& uri, const std::filesystem::path& path) {
  auto version = 11;

  // The io_context is required for all I/O
  net::io_context ioc;

  // The SSL context is required, and holds certificates
  ssl::context ctx{ssl::context::tlsv12_client};

  ctx.set_verify_mode(ssl::verify_none);

  // These objects perform our I/O
  tcp::resolver resolver{ioc};
  beast::ssl_stream<beast::tcp_stream> stream{ioc, ctx};

  // Set SNI Hostname (many hosts need this to handshake successfully)
  if (!SSL_set_tlsext_host_name(stream.native_handle(), uri.host.c_str())) {
    beast::error_code ec{static_cast<int>(::ERR_get_error()),
                         net::error::get_ssl_category()};
    throw beast::system_error{ec};
  }

  // Look up the domain name
  auto const results =
      resolver.resolve(uri.host, std::to_string(schemeToPort(uri.scheme)));

  // Make the connection on the IP address we get from a lookup
  beast::get_lowest_layer(stream).connect(results);

  // Perform the SSL handshake
  stream.handshake(ssl::stream_base::client);

  // Set up an HTTP GET request message
  http::request<http::string_body> req{http::verb::get, uri.target, version};
  req.set(http::field::host, uri.host);
  req.set(http::field::user_agent, BOOST_BEAST_VERSION_STRING);

  // Send the HTTP request to the remote host
  http::write(stream, req);

  // This buffer is used for reading and must be persisted
  beast::flat_buffer buffer;

  // Declare a container to hold the response
  // http::response<http::dynamic_body> res;
  http::response<http::vector_body<char>> res;

  // Receive the HTTP response
  http::read(stream, buffer, res);

  // Write the message to file
  {
    std::ofstream file{path, std::ios::binary | std::ios::out};
    const auto& data = res.body();
    std::copy(data.begin(), data.end(), std::ostreambuf_iterator(file));
  }

  // Gracefully close the stream
  beast::error_code ec;
  stream.shutdown(ec);
  if (ec == net::error::eof) {
    // Rationale:
    // http://stackoverflow.com/questions/25587403/boost-asio-ssl-async-shutdown-always-finishes-with-an-error
    ec = {};
  }
  if (ec.value() == 0x140000db) {
    //  TODO short read ??
    ec = {};
  }
  if (ec)
    throw beast::system_error{ec};
}

}  // namespace

void download(const Uri& uri, const std::filesystem::path& path) {
  switch (uri.scheme) {
    case Scheme::http:
      return httpDownload(uri, path);
    case Scheme::https:
      return httpsDownload(uri, path);
  }
}