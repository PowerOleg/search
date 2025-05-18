#include <boost/asio/io_context.hpp>
#include <memory>
#include <string>
#include "webpage.h"
#include "postgres_manager.h"
#include "indexer.h"
#include "file_manager.h"
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

void PrintConsole(std::vector<std::string> vector)
{
	for (const auto& value : vector)
	{
		std::cout << value << std::endl;
	}
}


boost::asio::io_context ioc;
bool working = true;
int recursion_count = 0;//отражает текущее значение рекурксии
int number_to_update_recursion = 1;



void UpdateRecursionLevel(const std::vector<std::string> &used_links, const std::vector<std::shared_ptr<Webpage>> &pages)
{
	if (used_links.size() >= number_to_update_recursion && pages.size() > 1)
	{
		int links_counter = 0;
		for (size_t i = 0; i < pages.size(); i++)
		{
			links_counter += pages.at(i)->GetLinks().size();
		}
		number_to_update_recursion = links_counter;
		recursion_count++;
	}
}

bool UpdateLinks(std::queue<std::string> &links_all, std::vector<std::shared_ptr<Webpage>> &pages, std::atomic_int &pages_count, std::vector<std::shared_ptr<Webpage>> &valid_pages)
{
	if (links_all.empty())
	{
		std::vector<std::string> links = pages.at(pages_count)->GetLinks();
		if (pages_count + 1 < pages.size() && links.size() == 0)
		{
			pages_count++;//this condition means it's bad page so just go to next page
		}
		if (links.size() > 0)
		{
			valid_pages.push_back(pages.at(pages_count));
			for (size_t i = 0; i < links.size(); i++)
			{
				links_all.push(links.at(i));//std::move(links.at(i)));
			}
			pages_count++;
			return true;
		}
			return false;
	}
	return true;
}


std::string GetLink(std::queue<std::string> &links_all, std::vector<std::string> &used_links, int &ret_flag)
{
	ret_flag = 1;
	std::string link = links_all.front();
	links_all.pop();
	std::chrono::milliseconds timespan(50);
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

void WriteWordsInDatabase(Postgres_manager &postgres, std::vector<std::shared_ptr<Webpage>> &pages, size_t &postgres_count, Config &config, long &word_number)
{
	std::shared_ptr<Webpage> page1 = pages.at(postgres_count++);
	std::string page_text = page1->GetPageText();
	Indexer page_indexer(page_text);
	std::vector<std::string> words = page_indexer.getWords();
	page_indexer.FilterSymbols(words);
	std::map<std::string, int> counted_words = page_indexer.Count(words);//std::move(words));
	postgres.Write(page1->GetPageUrl(), postgres_count, counted_words, word_number);
}

int main(int argc, char** argv)
{
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
	std::vector<std::shared_ptr<Webpage>> valid_pages;
	std::atomic_int pages_count = 0;

	
	size_t thread_quantity = 2;
	Thread_pool thread_pool(ioc, thread_quantity);
	size_t postgres_count = 0;
	long word_number = 1L;
	Postgres_manager postgres(config.sqlhost, config.sqlport, config.dbname, config.username, config.password);
	
	while (working)
	{
		if (!UpdateLinks(links_all, pages, pages_count, valid_pages))
		{
			continue;
		}

		int return_flag;
		std::string link = GetLink(links_all, used_links, return_flag);
		//std::cout << "links_all size: " << links_all.size() << " link: " << link << std::endl;
		if (return_flag == 3)
		{
			continue;
		}
		
		std::shared_ptr<Webpage> page = std::make_shared<Webpage>(ioc, link);
		pages.push_back(page);
		auto page_Load = [&page] { page->LoadPage(); };
		thread_pool.Enqueue(page_Load);

		if (valid_pages.size() > postgres_count)
		{
			WriteWordsInDatabase(postgres, valid_pages, postgres_count, config, word_number);
		}

		UpdateRecursionLevel(used_links, pages);
		if (recursion_count >= atoi(config.crawler_depth.c_str()))
		{
			working = false;
		}
	}

	return 0;
}