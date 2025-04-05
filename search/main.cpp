/*
#include <unordered_set>
#include <vector>
#include <thread>
#include <mutex>
#include <functional>
#include <boost/beast/core.hpp>
#include <boost/asio/thread_pool.hpp>
*/
#include "C:/cpp/boost_1_87Bin/libs/beast/example/common/root_certificates.hpp" //прописать свой путь
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
#include <vector>
#include <queue>
#include <unordered_set>
#include <thread>
#include <mutex>
#include <functional>
#include <iomanip>
#include <boost/bind.hpp>
#include <string>
#include <memory>

#include "webpage.h"
#include "postgres_manager.h"
#include "indexer.h"
#include "file_manager.h"

struct Config
{
	std::string sqlhost;//хост, на котором запущена база данных;
	std::string sqlport;//порт, на котором запущена база данных;
	std::string dbname;//название базы данных;
	std::string username;//имя пользователя для подключения к базе данных;
	std::string password;//пароль пользователя для подключения к базе данных;
	std::string url;//стартовая страница для программы «Паук»;
	std::string crowler_depth;//глубина рекурсии для программы «Паук»;
	std::string http_port;//порт для запуска программы - поисковика.
};

void PrintConsole(std::string text)
{
	std::cout << text << std::endl;
}


boost::asio::io_context ioc;

int main(int argc, char** argv)
{
	//system("chcp 1251");//setlocale(LC_ALL, "ru");
	Config config;
	File_manager file_manager("config.ini");
	file_manager.FillConfig(
		&config.sqlhost,
		&config.sqlport,
		&config.dbname,
		&config.username,
		&config.password,
		&config.url,
		&config.crowler_depth,
		&config.http_port);

	//std::vector<std::vector<std::string>> all_links;
	//std::unordered_set<std::string> ustUsed{ vUri.begin(), vUri.end() }; //для отработанных ссылок
	std::vector<std::string> vUri{ "https://mail.ru/" };//начальная ссылка
	std::vector<std::shared_ptr<Webpage>> pages;
	
	size_t thread_quantity = 2;
	boost::asio::thread_pool tpool{ thread_quantity };
	for (const auto& sUri : vUri)
	{
		std::shared_ptr<Webpage> page = std::make_shared<Webpage>(ioc, sUri);
		pages.push_back(page);

		auto page_Load = [&page] { page->LoadPage(); };
		boost::asio::post(tpool, page_Load);//  boost::asio::post() или boost::asio::dispatch()// boost::asio::post(tpool, std::bind(&Webpage::Load, this, std::cref(sUri), std::ref(links.back()))
	}
	tpool.join();

	std::string page_text = pages.at(0)->getPageText();
	std::vector<std::string> links = pages.at(0)->getLinks();

	Indexer page_indexer(page_text);
	std::vector<std::string> words = page_indexer.getWords();
	pages.at(0)->MoveWords(std::move(words));








	//Postgres_manager postgres("localhost", "5432", "dvdrental", "postgres", "106");
	//postgres.Test(page.getWordSet());



	return 0;
};