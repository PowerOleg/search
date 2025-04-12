#include "postgres_manager.h"

Postgres_manager::Postgres_manager(std::string host, std::string port, std::string dbname, std::string username, std::string password) : connection{ "host=" + host + " port=" + port + " dbname =" + dbname + " user = " + username + " password =" + password}
{
	std::cout << "Connection is successful" << std::endl;
	InitTables();
	Clean();
	this->connection.prepare("prepared_insert_word", "INSERT INTO Words(word) VALUES($1);");
	this->connection.prepare("prepared_insert_document", "INSERT INTO Documents(document) VALUES($1);");
	this->connection.prepare("prepared_insert_documents_words", "INSERT INTO Documents_words(document_id, word_id, quantity) VALUES($1, $2, $3);");
}

bool Postgres_manager::Clean()
{
	bool result = false;
	std::string table1 = "Documents";
	std::string table2 = "Words";
	std::string table3 = "Documents_words";
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
	return result;
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
	result = CreateTable(
		"Documents",
		"id SERIAL PRIMARY KEY, "
		"document VARCHAR(255) not null"	
		);
	result = CreateTable(
		"Words",
		"id SERIAL PRIMARY KEY, "
		"word VARCHAR(32) not null"
	);
	result = CreateTable(
		"Documents_words",
		"document_id int references Documents(id), "
		"word_id int references Words(id), "
		"quantity int not null, "
		"constraint pk primary key (document_id, word_id)"
	);
	return result;
}

bool Postgres_manager::Write(const std::string url, const std::map<std::string, int> &counted_words)
{
	bool result = false;
	//std::vector<std::string> words;
	if (url.length() > 255)
	{
		std::cout << "Ошибка! url длиннее 255 символов";
		return false;
	}

	try
	{
		pqxx::work tx{ connection };
		tx.exec_prepared("prepared_insert_document", url);//prepared statement
		
		for (const auto& word_and_quantity : counted_words)
		{
			std::string word = word_and_quantity.first;
			tx.exec_prepared("prepared_insert_word", word);//prepared statement
		}
		tx.commit();
	}
	catch (const std::exception& e)
	{
		std::cout << e.what() << std::endl;
	}


	std::string document_id = "1";
	
	try
	{
		pqxx::work tx2{ connection };
		long word_number = 1L;
		for (const auto& word_and_quantity : counted_words)
		{
			std::string word = word_and_quantity.first;
			std::string quantity = std::to_string(word_and_quantity.second);

		
			std::string word_id = std::to_string(word_number++);
			tx2.exec_prepared("prepared_insert_documents_words", document_id, word_id, quantity);//prepared statement
		}
		tx2.commit();
		result = true;
}
catch (const std::exception& e)
{
	std::cout << e.what() << std::endl;
}



//for debug
/*	pqxx::work tx3{connection};
	std::string table1 = "Documents";
	std::string table2 = "Words";
	std::string table3 = "Documents_words";

	pqxx::work tx{ this->connection };
	tx.exec("drop table " + table1 + " CASCADE;");
	tx.exec("drop table " + table2 + " CASCADE;");
	tx.exec("drop table " + table3 + " CASCADE;");
	tx3.commit();
	*/

	return result;
}
