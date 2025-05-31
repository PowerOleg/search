#pragma once

#include <string>
namespace crawler
{
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
}