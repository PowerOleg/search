#include <boost/beast/core.hpp>
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
#include "thread_pool.h"

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


boost::asio::io_context iocPage;
bool working = true;

bool UpdateLinks(std::queue<std::string>& links_all, std::vector<std::shared_ptr<Webpage>>& pages, std::atomic_int& pages_count)
{
	if (links_all.empty())
	{
		std::vector<std::string> links = pages.at(pages_count)->getLinks();
		if (pages_count + 1 < pages.size() && links.size() == 0)
		{
			pages_count++;
		}
		//std::vector<std::string> links = std::move(pages.at(pages_count++)->getLinks());//?std::move() оправданно?
		if (links.size() > 0)
		{
			for (size_t i = 0; i < links.size(); i++)
			{
				links_all.push(std::move(links.at(i)));
			}
			pages_count++;
			return true;
		}
			//std::cout << "empty" << std::endl;
			return false;
	}
	return true;
}


std::string GetLink(std::queue<std::string> &links_all, std::vector<std::string> &used_links, int &ret_flag)
{
	ret_flag = 1;
	std::string link = links_all.front();
	links_all.pop();
	std::cout << "links_all size: " << links_all.size() << std::endl;
	std::chrono::milliseconds timespan(200);
	std::this_thread::sleep_for(timespan);
	std::regex regex_pattern{ "^(?:(https?)://)([^/]+)(/.*)?" };
	std::smatch match;
	if (link == "" || !std::regex_match(link, match, regex_pattern))
	{
		ret_flag = 3; 
		return link;
	}
	used_links.push_back(link);
	return link;
}

void ThreadPoolImplementation2(std::queue<std::string>& links_all, std::vector<std::shared_ptr<Webpage>>& pages, std::atomic_int& pages_count, std::vector<std::string>& used_links)
{
	size_t thread_quantity = 2;
	Thread_pool thread_pool(thread_quantity);

	
	while (working)
	{
		if (!UpdateLinks(links_all, pages, pages_count))
		{
			continue;
		}
		int return_flag;
		std::string link = GetLink(links_all, used_links, return_flag);
		if (return_flag == 3) continue;

		std::shared_ptr<Webpage> page = std::make_shared<Webpage>(iocPage, link);
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