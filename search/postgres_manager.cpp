#include "postgres_manager.h"

Postgres_manager::Postgres_manager(std::string host, std::string port, std::string dbname, std::string username, std::string password) : connection{ "host=" + host + " port=" + port + " dbname =" + dbname + " user = " + username + " password =" + password}
{}

bool Postgres_manager::Write(std::string url, std::vector<std::string> words)
{
	bool result = false;

	try
	{
		pqxx::work tx2{ connection };
		std::string result = tx2.query_value<std::string>(
			"SELECT a.first_name FROM actor a where a.actor_id = 3;"
		);
		tx2.commit();
		std::cout << "select query" << std::endl;
	}
	catch (const std::exception& e)
	{
		std::cout << e.what() << std::endl;
	}



	return result;
}
