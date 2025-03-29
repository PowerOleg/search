#pragma once

#include <boost/beast/core.hpp>
#include <boost/beast/http.hpp>
#include <boost/beast/version.hpp>
#include <boost/asio/connect.hpp>
#include <boost/asio/ip/tcp.hpp>
#include <boost/beast/websocket.hpp>
#include <cstdlib>
#include "common.h"

namespace beast = boost::beast;     // from <boost/beast.hpp>
namespace http = beast::http;       // from <boost/beast/http.hpp>
namespace net = boost::asio;        // from <boost/asio.hpp>
using tcp = net::ip::tcp;           // from <boost/asio/ip/tcp.hpp>
namespace websocket = beast::websocket; // from <boost/beast/websocket.hpp>

class Crawler
{
public:
	Crawler(std::string host_, const std::string target_);
	int SimpleHttpRequest();
	int HttpWebSocketRequest();

private:
	std::string host;
    const std::string target;
	const std::string port = "80";
	const int version = 11;
};

