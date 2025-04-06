#include "postgres_manager.h"

Postgres_manager::Postgres_manager(std::string host, std::string port, std::string dbname, std::string username, std::string password) : connection{ "host=" + host + " port=" + port + " dbname =" + dbname + " user = " + username + " password =" + password}
{
	InitTables();
	Clean();
}

bool Postgres_manager::Clean()
{
	bool result = false;
	std::string table1 = "Documents";
	std::string table2 = "Words";
	std::string table3 = "DocumentsWords";
	try
	{
		pqxx::work tx{ this->connection };
		tx.exec("TRUNCATE " + table1 + ", " + table2 + ", " + table3 + ";");
		tx.commit();
		result = true;
	}
	catch (const std::exception& e)
	{
		std::cout << e.what() << std::endl;
	}
	return false;
}

bool Postgres_manager::CreateTable(std::string tablename, std::string param)
{
	bool result = false;
	try
	{
		pqxx::work tx{ this->connection };
		tx.exec("CREATE TABLE IF NOT EXISTS " + tablename + " (" + param + ");");
		tx.commit();
		result = true;
	}
	catch (const std::exception& e)
	{
		std::cout << e.what() << std::endl;
	}
	return result;
}

bool Postgres_manager::InitTables()
{
	bool result = false;
	try
	{
		CreateTable(
			"Documents",
			"id SERIAL PRIMARY KEY, "
			"firstname varchar(255) NOT NULL, "
			"secondname varchar(255) NOT NULL"
		);
		std::cout << "A table Documents has created" << std::endl;
		CreateTable(
			"Words",
			"id SERIAL PRIMARY KEY, "
			"email varchar(255), "
			"phone_number varchar(255)"
			//"client_id int references Client(id)"
		);
		std::cout << "A table Words has been created" << std::endl;
		CreateTable(
			"Documents_words",
			"id SERIAL PRIMARY KEY, "
			"email varchar(255), "
			"phone_number varchar(255)"
		);
		std::cout << "A table Documents_words has been created" << std::endl;
		result = true;
	}
	catch (const std::exception& e)
	{
		std::cout << e.what() << std::endl;
	}
	return result;
}

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
