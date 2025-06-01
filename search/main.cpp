#include <boost/asio/io_context.hpp>
#include <memory>
#include <thread>
#include <mutex>
#include <string>
#include "webpage.h"
#include "postgres_manager.h"
#include "file_manager.h"
#include "thread_pool.h"
#include "link.h"
#include "config.h"

#pragma execution_character_set("utf-8")

using namespace crawler;

boost::asio::io_context ioc;
size_t thread_quantity = 2;
bool working = true;
int recursion_count = 0;//отражает текущее значение рекурксии
int number_to_update_recursion = 1;
std::mutex links_all_mutex;

std::shared_ptr<Link> GetLink(std::queue<std::shared_ptr<Link>> &links_all, int &ret_flag)
{
	std::scoped_lock lock(links_all_mutex);
	ret_flag = 1;
	std::shared_ptr<Link> link{ links_all.front() };
	links_all.pop();

	std::regex regex_pattern{ "^(?:(https?)://)([^/]+)(/.*)?" };
	std::smatch match;
	if (link->string_link == "" || !std::regex_match(link->string_link, match, regex_pattern))
	{
		ret_flag = 3; 
		return link;
	}
	return link;
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
	
	std::queue<std::shared_ptr<Link>> links_all;
	links_all.push(std::make_shared<Link>(config.url, 1));//начальная ссылка
	Thread_pool thread_pool(ioc, thread_quantity);
	Postgres_manager postgres(config.sqlhost, config.sqlport, config.dbname, config.username, config.password);
	
	while (working)
	{
		if (!links_all.empty())
		{
			int return_flag;
			std::shared_ptr<Link> link = GetLink(links_all, return_flag);
			if (return_flag == 3)
			{
				continue;
			}
			if (link->recursion_level > atoi(config.crawler_depth.c_str()))
			{
				working = false;
				break;
			}
			std::shared_ptr<Webpage> page = std::make_shared<Webpage>(ioc, link->string_link, links_all_mutex, link->recursion_level, postgres);
			auto page_Load = [page, &links_all] { page->LoadPage(links_all); };
			thread_pool.Enqueue(page_Load);
		}
		else {}//if delete this line the programm doesn't work in proper way

	}
	
	std::chrono::milliseconds timespan(30000);
	std::this_thread::sleep_for(timespan);
	return 0;
}