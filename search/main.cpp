﻿#include <boost/beast/core.hpp>
#include <boost/asio.hpp>
#include <boost/thread.hpp>
#include <boost/asio/thread_pool.hpp>
#include <boost/asio/io_context.hpp>
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
#include "thread_pool.h"
#include "main.h"

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

void ThreadPoolGetPage(std::string &link, std::vector<std::shared_ptr<Webpage>> &pages, boost::asio::thread_pool &tpool)
{
	std::shared_ptr<Webpage> page = std::make_shared<Webpage>(ioc, link);
	pages.push_back(page);

	auto page_Load = [&page] { page->LoadPage(); };
	boost::asio::post(tpool, page_Load);//  boost::asio::post() или boost::asio::dispatch()// boost::asio::post(tpool, std::bind(&Webpage::Load, this, std::cref(sUri), std::ref(links.back()))
}


void UpdateLinks(std::queue<std::string>& links_all, std::vector<std::shared_ptr<Webpage>>& pages, std::atomic_int& pages_count, size_t& сountdown, int& retFlag)
{
	retFlag = 1;
	if (links_all.empty())
	{
		std::vector<std::string> links = pages.at(pages_count)->getLinks();
		//std::vector<std::string> links = std::move(pages.at(pages_count++)->getLinks());//?std::move() оправданно?
		if (links.size() > 0)
		{
			for (size_t i = 0; i < links.size(); i++)
			{
				links_all.push(std::move(links.at(i)));
			}
			pages_count++;
		}
		else
		{
			std::cout << "empty" << std::endl;
			//std::chrono::milliseconds timespan(500);
			//std::this_thread::sleep_for(timespan);
			сountdown--;
		}
		{ retFlag = 3; return; };
	}
}

void ThreadPoolImplementation1(std::queue<std::string>& links_all, std::vector<std::shared_ptr<Webpage>>& pages, std::atomic_int& pages_count, std::vector<std::string>& used_links)
{
	size_t thread_quantity = 2;
	boost::asio::thread_pool tpool{ thread_quantity };
	size_t сountdown = 10000;
	while (сountdown > 0)
	{

		int retFlag;
		UpdateLinks(links_all, pages, pages_count, сountdown, retFlag);//этот блок кода вызывает exception у tpool
		if (retFlag == 3) continue;


		std::string link = links_all.front();
		if (link == "")
		{
			continue;
		}
		links_all.pop();
		used_links.push_back(link);

		ThreadPoolGetPage(link, pages, tpool);

	}
	tpool.join();
};

void ThreadPoolImplementation2(std::queue<std::string>& links_all, std::vector<std::shared_ptr<Webpage>>& pages, std::atomic_int& pages_count, std::vector<std::string>& used_links)
{
	size_t thread_quantity = 2;
	Thread_pool thread_pool(ioc, thread_quantity);

	size_t сountdown = 10000;
	while (сountdown > 0)
	{
		int retFlag;
		UpdateLinks(links_all, pages, pages_count, сountdown, retFlag);
		if (retFlag == 3) continue;


		std::string link = links_all.front();
		if (link == "")
		{
			continue;
		}
		links_all.pop();
		used_links.push_back(link);

		std::shared_ptr<Webpage> page = std::make_shared<Webpage>(ioc, link);
		pages.push_back(page);
		auto page_Load = [&page] { page->LoadPage(); };
		
		thread_pool.Enqueue(page_Load);
	}
	
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
	
	
	//ThreadPoolImplementation1(links_all, pages, pages_count, used_links);
	ThreadPoolImplementation2(links_all, pages, pages_count, used_links);


	std::string page_text = pages.at(0)->getPageText();
	std::vector<std::string> page_links = pages.at(0)->getLinks();

	Indexer page_indexer(page_text);
	std::vector<std::string> words = page_indexer.getWords();
	std::shared_ptr<Webpage> page1 = pages.at(0);
	page1->MoveWords(std::move(words));
	

	std::vector<std::string> words1 = page1->getWords();
	page_indexer.FilterSymbols(words1);
	std::map<std::string, int> counted_words = page_indexer.Count(words1);

	//Postgres_manager postgres("localhost", "5432", "pages", "postgres", "106");
	//postgres.Write("https://mail.ru/", counted_words);

	return 0;
}


//File_manager file_manager_test_text("test.txt");//debug
	//std::vector<std::string> words = file_manager_test_text.SimpleRead();//debug
	//Indexer page_indexer("");//debug