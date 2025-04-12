/*
#include <unordered_set>
#include <vector>
#include <thread>
#include <mutex>
#include <functional>
#include <boost/beast/core.hpp>
#include <boost/asio/thread_pool.hpp>
*/

/*
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

*/

#include <boost/beast/core.hpp>
#include <boost/asio/thread_pool.hpp>
#include <string>
#include <map>
#include <memory>
#include <clocale>
#include <locale>
#include "webpage.h"
#include "postgres_manager.h"
#include "indexer.h"
#include "file_manager.h"
#include <cstdlib>
#include <thread>
#include <chrono>

#pragma execution_character_set("utf-8")

struct Config
{
	std::string sqlhost;//хост, на котором запущена база данных;
	std::string sqlport;//порт, на котором запущена база данных;
	std::string dbname;//название базы данных;
	std::string username;//имя пользователя для подключения к базе данных;
	std::string password;//пароль пользователя для подключения к базе данных;
	std::string url;//стартовая страница для программы «Паук»;
	std::string crawler_depth;//глубина рекурсии для программы «Паук»;
	std::string http_port;//порт для запуска программы - поисковика.
};

void PrintConsole(std::string text)
{
	std::cout << text << std::endl;
}


boost::asio::io_context ioc;

void ThreadPoolGetPage(std::string& link, std::vector<std::shared_ptr<Webpage>>& pages, boost::asio::thread_pool& tpool)
{
	std::shared_ptr<Webpage> page = std::make_shared<Webpage>(ioc, link);
	pages.push_back(page);

	auto page_Load = [&page] { page->LoadPage(); };
	boost::asio::post(tpool, page_Load);//  boost::asio::post() или boost::asio::dispatch()// boost::asio::post(tpool, std::bind(&Webpage::Load, this, std::cref(sUri), std::ref(links.back()))
}

int main(int argc, char** argv)
{
	//setlocale(LC_ALL, "");
	//system("chcp 1251");
	//std::locale::global(std::locale(""));
	//system("chcp 1251 > nul");
	//system("chcp utf-8 > nul");
	//setlocale(LC_ALL, "ru");

	SetConsoleCP(CP_UTF8);
	SetConsoleOutputCP(CP_UTF8);


	Config config;
	File_manager file_manager("config.ini");
	file_manager.FillConfig(
		&config.sqlhost,
		&config.sqlport,
		&config.dbname,
		&config.username,
		&config.password,
		&config.url,
		&config.crawler_depth,
		&config.http_port);
	
	std::queue<std::string> links_all;
	std::vector<std::string> used_links;//для отработанных ссылок
	links_all.push(config.url);//начальная ссылка
	std::vector<std::shared_ptr<Webpage>> pages;
	std::atomic_int pages_count = 0;
	
	
	size_t thread_quantity = 2;
	boost::asio::thread_pool tpool{ thread_quantity };
	size_t сountdown = 1;
	while (сountdown > 0)
	{
		if (links_all.empty())
		{
			
			if (pages.size() > pages_count)
			{
				std::vector<std::string> links = std::move(pages.at(pages_count++)->getLinks());//?оправданно?
				for (size_t i = 0; i < links.size(); i++)
				{
					links_all.push(std::move(links.at(i)));
				}
			} 
			else
			{
				std::cout << "empty" << std::endl;
				std::chrono::milliseconds timespan(500);
				std::this_thread::sleep_for(timespan);
				сountdown--;
			}
			continue;
		}
		std::string link = links_all.front();
		if (link == "")
		{
			continue;
		}
		links_all.pop();
		used_links.push_back(link);

		ThreadPoolGetPage(link, pages, tpool);//где-то тут падает
	}
	tpool.join();//гдето-тут падает

	std::string page_text = pages.at(0)->getPageText();
	Indexer page_indexer(page_text);
	std::vector<std::string> words = page_indexer.getWords();
	std::shared_ptr<Webpage> page1 = pages.at(0);
	page1->MoveWords(std::move(words));
	


	

	std::vector<std::string> words1 = page1->getWords();
	page_indexer.FilterSymbols(words1);
	std::map<std::string, int> counted_words = page_indexer.Count(words1);
	Postgres_manager postgres("localhost", "5432", "pages", "postgres", "106");
	postgres.Write("https://mail.ru/", counted_words);

	return 0;
};

//File_manager file_manager_test_text("test.txt");//debug
	//std::vector<std::string> words = file_manager_test_text.SimpleRead();//debug
	//Indexer page_indexer("");//debug