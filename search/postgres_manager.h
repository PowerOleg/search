#pragma once
#include <pqxx/pqxx>
#include <string>
#include <vector>
#include <map>
#include <set>
#include <iostream>
#include <mutex>

class Postgres_manager
{
public:
	Postgres_manager(const std::string host, const std::string port, const std::string dbname, const std::string username, const std::string password);

	bool Write(const std::string url, const std::map<std::string, int>& counted_words);
	std::vector<std::string> SelectUrls(const std::string& word, const std::string& quantity);
	void AddLinkInWrittenSet(const std::string link);
	bool IsLinkDuplicate(const std::string link);
	
private:
	bool Clean();
	bool CreateTable(std::string tablename, std::string param);
	bool InitTables();
	pqxx::connection connection;

	std::string last_word_id_last_iteration = "initial";
	std::set<std::string> links_written_in_database;
	std::mutex postgres_mutex;
};

