#pragma once
#include <pqxx/pqxx>
//#include <Windows.h>
#include "common.h"
#include <vector>
#include <map>
#include <string>
//using namespace pqxx;

class Postgres_manager
{
public:
	Postgres_manager(std::string host, std::string port, std::string dbname, std::string username, std::string password);
	
	bool Write(std::string url, std::map<std::string, int> words);

	void SelectTest()
	{
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
	}

private:
	bool Clean();
	bool CreateTable(std::string tablename, std::string param);
	bool InitTables();
	pqxx::connection connection;
};

