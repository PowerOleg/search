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
#include "config.h"


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

	Webpage(boost::asio::io_context &ioc_, const std::string url_, std::mutex &m_, int recursion_level_, Config config, Postgres_manager &postgres_manager);

	void LoadPage(std::queue<std::shared_ptr<Link>> &links_all);
	std::string GetPageText() { return page_text; };
	std::vector<std::string> GetLinks() { return page_links; };
	void MoveWords(std::vector<std::string>&& words_) { this->words = std::move(words_); };
	std::vector<std::string> GetWords() { return words; };
	std::string GetPageUrl() { return url; };

private:
	std::queue<std::shared_ptr<Link>> LoadHttp(const std::smatch &match);
	std::queue<std::shared_ptr<Link>> LoadHttps(const std::smatch &match);
	std::vector<std::string> FindLinks(std::string const sBody);
	void AbsLinks(const std::vector<std::string>& init_links, std::queue<std::shared_ptr<Link>> &abs_links);
	void PushQueue(std::queue<std::shared_ptr<Link>> &source, std::queue<std::shared_ptr<Link>> &destination);
	void WriteWordsInDatabase(Postgres_manager& postgres, std::vector<std::shared_ptr<Webpage>>& pages, size_t& postgres_count, Config& config, long& word_number);

private:
	std::string url;
	std::string host;
    //const std::string target;
	const std::string port = "80";
	const int version = 11;

	std::regex regex_pattern{ "^(?:(https?)://)([^/]+)(/.*)?" };
	std::mutex mtx;
	boost::asio::io_context &ioc;

	std::string page_text;
	std::vector<std::string> page_links;
	std::vector<std::string> words;
	std::mutex &m;
	int recursion_level = 0;


	//Config config;
	Postgres_manager &postgres;
};

