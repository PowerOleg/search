#pragma once
//#include "C:/cpp/boost_1_87Bin/libs/beast/example/common/root_certificates.hpp" //��������� ���� ����
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


namespace beast = boost::beast;     // from <boost/beast.hpp>
namespace http = beast::http;       // from <boost/beast/http.hpp>
namespace net = boost::asio;        // from <boost/asio.hpp>
using tcp = net::ip::tcp;           // from <boost/asio/ip/tcp.hpp>
namespace websocket = beast::websocket; // from <boost/beast/websocket.hpp>
//using namespace boost::asio;

class Webpage
{
public:
	Webpage(Webpage const&) = delete;
	Webpage& operator=(Webpage const&) = delete;

	Webpage(boost::asio::io_context &ioc_, const std::string url_, std::mutex &m_);

	void LoadPage(std::queue<std::string> &links_all);
	std::string GetPageText() { return page_text; };
	std::vector<std::string> GetLinks() { return page_links; };
	void MoveWords(std::vector<std::string>&& words_) { this->words = std::move(words_); };
	std::vector<std::string> GetWords() { return words; };
	std::string GetPageUrl() { return url; };
	bool IsValid();
	void SetValid();

private:
	std::queue<std::string> LoadHttp(const std::smatch& match);
	std::queue<std::string> LoadHttps(std::smatch const& match);
	std::vector<std::string> FindLinks(std::string const& sBody);
	void AbsLinks(const std::vector<std::string>& init_links, std::queue<std::string> &abs_links);
	void PushQueue(std::queue<std::string> &source, std::queue<std::string> &destination);

private:
	std::string url;
	std::string host;
    const std::string target;
	const std::string port = "80";
	const int version = 11;

	std::regex regex_pattern{ "^(?:(https?)://)([^/]+)(/.*)?" };
	std::mutex mtx;
	boost::asio::io_context &ioc;

	std::string page_text;
	std::vector<std::string> page_links;
	std::vector<std::string> words;
	bool is_valid_page = false;
	std::mutex &m;
};

