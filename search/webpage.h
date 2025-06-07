#pragma once
//#include "C:/cpp/boost_1_87Bin/libs/beast/example/common/root_certificates.hpp" //прописать свой путь
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
#include <fstream>
#include <regex>
#include <queue>
#include <vector>
#include <thread>
#include <mutex>
#include <functional>
#include <iomanip>
#include <unordered_set>
#include "gumbo.h"
#include "link.h"
#include "postgres_manager.h"
#include "indexer.h"


namespace beast = boost::beast;     // from <boost/beast.hpp>
namespace http = beast::http;       // from <boost/beast/http.hpp>
namespace net = boost::asio;        // from <boost/asio.hpp>
using tcp = net::ip::tcp;           // from <boost/asio/ip/tcp.hpp>
namespace websocket = beast::websocket; // from <boost/beast/websocket.hpp>
using namespace crawler;
//using namespace boost::asio;

class Webpage
{
public:
	Webpage(Webpage const&) = delete;
	Webpage& operator=(Webpage const&) = delete;
	Webpage(boost::asio::io_context &ioc_, const std::string url_, std::mutex &links_all_mutex_, size_t recursion_level_, Postgres_manager &postgres_manager, const size_t crawler_depth_stop_);

	void LoadPage(std::queue<std::shared_ptr<Link>> &links_all);
	std::string GetPageText() { return page_text; };
	std::string GetPageUrl() { return url; };

private:
	std::queue<std::shared_ptr<Link>> LoadHttp(const std::smatch &match);
	std::queue<std::shared_ptr<Link>> LoadHttps(const std::smatch &match);
	std::vector<std::string> FindLinks(std::string const sBody);
	void AbsLinks(const std::vector<std::string>& init_links, std::queue<std::shared_ptr<Link>> &abs_links);
	void PushQueue(std::queue<std::shared_ptr<Link>> &source, std::queue<std::shared_ptr<Link>> &destination);
	void WriteWordsInDatabase();

private:
	std::string url;
	std::string host;
	std::regex regex_pattern{ "^(?:(https?)://)([^/]+)(/.*)?" };
	std::mutex mtx;
	boost::asio::io_context &ioc;

	std::string page_text;
	std::mutex &links_all_mutex;
	size_t crawler_depth = 0;
	size_t crawler_depth_stop;
	Postgres_manager &postgres;
};

