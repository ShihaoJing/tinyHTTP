//
// Created by fanshiliang on 2017/6/22.
//

#ifndef SLASH_REPLY_H
#define SLASH_REPLY_H

#include <string>
#include <vector>
#include <unordered_map>
#include <boost/asio.hpp>
#include "header.h"

namespace HTTP {
namespace Server {

/// A reply to be sent to a client.
struct response
{
  /// The status of the reply.
  enum status_type
  {
    ok = 200,
    created = 201,
    accepted = 202,
    no_content = 204,
    multiple_choices = 300,
    moved_permanently = 301,
    moved_temporarily = 302,
    not_modified = 304,
    bad_request = 400,
    unauthorized = 401,
    forbidden = 403,
    not_found = 404,
    internal_server_error = 500,
    not_implemented = 501,
    bad_gateway = 502,
    service_unavailable = 503
  } status;

  /// The headers to be included in the reply.
  std::vector<header> headers;

  /// The content to be sent in the reply.
  std::string content;

  /// Convert the reply into a vector of buffers. The buffers do not own the
  /// underlying memory blocks, therefore the reply object must remain valid and
  /// not be changed until the write operation has completed.
  std::vector<boost::asio::const_buffer> to_buffers();

  /// Get a stock reply.
  static response stock_reply(status_type status);
};

namespace status_strings {

const std::string ok =
    "HTTP/1.0 200 OK\r\n";
const std::string created =
    "HTTP/1.0 201 Created\r\n";
const std::string accepted =
    "HTTP/1.0 202 Accepted\r\n";
const std::string no_content =
    "HTTP/1.0 204 No Content\r\n";
const std::string multiple_choices =
    "HTTP/1.0 300 Multiple Choices\r\n";
const std::string moved_permanently =
    "HTTP/1.0 301 Moved Permanently\r\n";
const std::string moved_temporarily =
    "HTTP/1.0 302 Moved Temporarily\r\n";
const std::string not_modified =
    "HTTP/1.0 304 Not Modified\r\n";
const std::string bad_request =
    "HTTP/1.0 400 Bad Request\r\n";
const std::string unauthorized =
    "HTTP/1.0 401 Unauthorized\r\n";
const std::string forbidden =
    "HTTP/1.0 403 Forbidden\r\n";
const std::string not_found =
    "HTTP/1.0 404 Not Found\r\n";
const std::string internal_server_error =
    "HTTP/1.0 500 Internal Server Error\r\n";
const std::string not_implemented =
    "HTTP/1.0 501 Not Implemented\r\n";
const std::string bad_gateway =
    "HTTP/1.0 502 Bad Gateway\r\n";
const std::string service_unavailable =
    "HTTP/1.0 503 Service Unavailable\r\n";

boost::asio::const_buffer to_buffer(response::status_type status)
{
  switch (status)
  {
    case response::ok:
      return boost::asio::buffer(ok);
    case response::created:
      return boost::asio::buffer(created);
    case response::accepted:
      return boost::asio::buffer(accepted);
    case response::no_content:
      return boost::asio::buffer(no_content);
    case response::multiple_choices:
      return boost::asio::buffer(multiple_choices);
    case response::moved_permanently:
      return boost::asio::buffer(moved_permanently);
    case response::moved_temporarily:
      return boost::asio::buffer(moved_temporarily);
    case response::not_modified:
      return boost::asio::buffer(not_modified);
    case response::bad_request:
      return boost::asio::buffer(bad_request);
    case response::unauthorized:
      return boost::asio::buffer(unauthorized);
    case response::forbidden:
      return boost::asio::buffer(forbidden);
    case response::not_found:
      return boost::asio::buffer(not_found);
    case response::internal_server_error:
      return boost::asio::buffer(internal_server_error);
    case response::not_implemented:
      return boost::asio::buffer(not_implemented);
    case response::bad_gateway:
      return boost::asio::buffer(bad_gateway);
    case response::service_unavailable:
      return boost::asio::buffer(service_unavailable);
    default:
      return boost::asio::buffer(internal_server_error);
  }
}

} // namespace status_strings

namespace misc_strings {

const char name_value_separator[] = { ':', ' ' };
const char crlf[] = { '\r', '\n' };

} // namespace misc_strings

std::vector<boost::asio::const_buffer> response::to_buffers()
{
  std::vector<boost::asio::const_buffer> buffers;
  buffers.push_back(status_strings::to_buffer(status));
  for (int i = 0; i < headers.size(); i++)
  {
    buffers.push_back(boost::asio::buffer(headers[i].key));
    buffers.push_back(boost::asio::buffer(misc_strings::name_value_separator, std::strlen(misc_strings::name_value_separator)));
    buffers.push_back(boost::asio::buffer(headers[i].value));
    buffers.push_back(boost::asio::buffer(misc_strings::crlf, std::strlen(misc_strings::crlf)));
  }
  buffers.push_back(boost::asio::buffer(misc_strings::crlf, std::strlen(misc_strings::crlf)));
  buffers.push_back(boost::asio::buffer(content));
  return buffers;
}

namespace stock_replies {

const char ok[] = "";
const char created[] =
    "<html>"
        "<head><title>Created</title></head>"
        "<body><h1>201 Created</h1></body>"
        "</html>";
const char accepted[] =
    "<html>"
        "<head><title>Accepted</title></head>"
        "<body><h1>202 Accepted</h1></body>"
        "</html>";
const char no_content[] =
    "<html>"
        "<head><title>No Content</title></head>"
        "<body><h1>204 Content</h1></body>"
        "</html>";
const char multiple_choices[] =
    "<html>"
        "<head><title>Multiple Choices</title></head>"
        "<body><h1>300 Multiple Choices</h1></body>"
        "</html>";
const char moved_permanently[] =
    "<html>"
        "<head><title>Moved Permanently</title></head>"
        "<body><h1>301 Moved Permanently</h1></body>"
        "</html>";
const char moved_temporarily[] =
    "<html>"
        "<head><title>Moved Temporarily</title></head>"
        "<body><h1>302 Moved Temporarily</h1></body>"
        "</html>";
const char not_modified[] =
    "<html>"
        "<head><title>Not Modified</title></head>"
        "<body><h1>304 Not Modified</h1></body>"
        "</html>";
const char bad_request[] =
    "<html>"
        "<head><title>Bad Request</title></head>"
        "<body><h1>400 Bad Request</h1></body>"
        "</html>";
const char unauthorized[] =
    "<html>"
        "<head><title>Unauthorized</title></head>"
        "<body><h1>401 Unauthorized</h1></body>"
        "</html>";
const char forbidden[] =
    "<html>"
        "<head><title>Forbidden</title></head>"
        "<body><h1>403 Forbidden</h1></body>"
        "</html>";
const char not_found[] =
    "<html>"
        "<head><title>Not Found</title></head>"
        "<body><h1>404 Not Found</h1></body>"
        "</html>";
const char internal_server_error[] =
    "<html>"
        "<head><title>Internal Server Error</title></head>"
        "<body><h1>500 Internal Server Error</h1></body>"
        "</html>";
const char not_implemented[] =
    "<html>"
        "<head><title>Not Implemented</title></head>"
        "<body><h1>501 Not Implemented</h1></body>"
        "</html>";
const char bad_gateway[] =
    "<html>"
        "<head><title>Bad Gateway</title></head>"
        "<body><h1>502 Bad Gateway</h1></body>"
        "</html>";
const char service_unavailable[] =
    "<html>"
        "<head><title>Service Unavailable</title></head>"
        "<body><h1>503 Service Unavailable</h1></body>"
        "</html>";

std::string to_string(response::status_type status)
{
  switch (status)
  {
    case response::ok:
      return ok;
    case response::created:
      return created;
    case response::accepted:
      return accepted;
    case response::no_content:
      return no_content;
    case response::multiple_choices:
      return multiple_choices;
    case response::moved_permanently:
      return moved_permanently;
    case response::moved_temporarily:
      return moved_temporarily;
    case response::not_modified:
      return not_modified;
    case response::bad_request:
      return bad_request;
    case response::unauthorized:
      return unauthorized;
    case response::forbidden:
      return forbidden;
    case response::not_found:
      return not_found;
    case response::internal_server_error:
      return internal_server_error;
    case response::not_implemented:
      return not_implemented;
    case response::bad_gateway:
      return bad_gateway;
    case response::service_unavailable:
      return service_unavailable;
    default:
      return internal_server_error;
  }
}

} // namespace stock_replies

response response::stock_reply(response::status_type status)
{
  response rep;
  rep.status = status;
  rep.content = stock_replies::to_string(status);
  rep.headers.resize(2);
  rep.headers[0].key = "Content-Length";
  rep.headers[0].value = std::to_string(rep.content.size());
  rep.headers[1].key = "Content-Type";
  rep.headers[1].value = "text/html";
  return rep;
}

} // namespace server
} // namespace http

#endif //SLASH_REPLY_H
