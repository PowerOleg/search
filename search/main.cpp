#include "file_manager.h"
#include "crowler.h"
#include "postgres_manager.h"
/* LAYOUT
	1) init
	{
		1 подключаемся к sql
		2 boost httpclient
		3 boost server
		4 xml?
	}
	2) запрашиваем одно слово для поиска (в дальнейшем поиск 4 слов)
	3) вносим стартовую страницу в queue
	4) запускаем thread_pool тасков
	 таска делает
	 {
		1 индексируем сайт - это заносим данные в sql
		2 ищем ссылки и вносим в queue
	 }

	thread_pool обрабатывает внесенные таски while queue != 0
	5) вывод результата. статическая страница с первыми 10 самыми релевантными ссылоками на веб-страницы
	*/

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

int main(int argc, char** argv)
{
	setlocale(LC_ALL, "ru");
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


	Postgres_manager postgres("localhost", "5432", "dvdrental", "postgres", "106");
	//postgres.Test();


	std::string host = "httpbin.org";
	std::string target = "/get";
	Crowler crowler(host, target);
	

	crowler.HttpRequest();

	return 0;
}