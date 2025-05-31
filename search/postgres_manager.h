#pragma once
#include <pqxx/pqxx>
#include <string>
#include <vector>
#include <map>
#include <iostream>
#include <mutex>

class Postgres_manager
{
public:
	Postgres_manager(const std::string host, const std::string port, const std::string dbname, const std::string username, const std::string password);

	bool Write(const std::string url, const std::map<std::string, int>& counted_words);
	std::vector<std::string> SelectUrls(const std::string& word, const std::string& quantity);
	
private:
	bool Clean();
	bool CreateTable(std::string tablename, std::string param);
	bool InitTables();
	pqxx::connection connection;

	std::string last_word_id_last_iteration = "initial";
	std::mutex postgres_mutex;
};

