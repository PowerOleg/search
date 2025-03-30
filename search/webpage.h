#pragma once
#include "C:/cpp/boost_1_87Bin/libs/beast/example/common/root_certificates.hpp" //��������� ���� ����
#include <boost/beast/core.hpp>
#include <boost/beast/http.hpp>
#include <boost/beast/version.hpp>
#include <boost/asio/connect.hpp>
#include <boost/asio/ip/tcp.hpp>
#include <boost/asio/ssl/error.hpp>
#include <boost/asio/ssl/stream.hpp>
#include <boost/asio/thread_pool.hpp>
#include <cstdlib>
#include <iostream>
#include <string>
#include <fstream>
#include <regex>
//#include <queue>
#include <vector>
#include <thread>
#include <mutex>
#include <functional>
#include <iomanip>
//#include <unordered_set>
#include "gumbo.h"
/*#include <boost/beast/core.hpp>
#include <boost/beast/http.hpp>
#include <boost/beast/version.hpp>
#include <boost/asio/connect.hpp>
#include <boost/asio/ip/tcp.hpp>
#include <boost/beast/websocket.hpp>
#include <cstdlib>
#include "common.h"*/

namespace beast = boost::beast;     // from <boost/beast.hpp>
namespace http = beast::http;       // from <boost/beast/http.hpp>
namespace net = boost::asio;        // from <boost/asio.hpp>
using tcp = net::ip::tcp;           // from <boost/asio/ip/tcp.hpp>
namespace websocket = beast::websocket; // from <boost/beast/websocket.hpp>

class Webpage
{
public:
	Webpage(Webpage const&) = delete;
	Webpage& operator=(Webpage const&) = delete;

	Webpage(boost::asio::io_context& ioc_, const std::string url_);
	int SimpleHttpRequest();
	//int HttpWebSocketRequest();

	void LoadPage(const std::string& sUri, std::vector<std::string>& vres);
	std::string getPagePlainText() { return page_plain_text; };
	std::vector<std::string> getLinks() { return vLinks; };

private:
	std::vector<std::string> LoadHttp(const std::smatch& match);
	std::vector<std::string> LoadHttps(std::smatch const& match);
	std::vector<std::string> FindLinks(std::string const& sBody);

private:
	std::string url;
	std::string host;
    const std::string target;
	const std::string port = "80";
	const int version = 11;

	std::regex rUri{ "^(?:(https?)://)([^/]+)(/.*)?" };
	std::mutex mtx;
	boost::asio::io_context& ioc;

	std::string page_plain_text;
	std::vector<std::string> vLinks;
};

