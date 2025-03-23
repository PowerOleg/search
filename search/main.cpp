#include "file_manager.h"
#include "crowler.h"
#include "postgres_manager.h"

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
	//crowler.Mapping(config.url);
	Crowler crowler(host, target);
	//crowler.HttpRequest();

	return 0;
}