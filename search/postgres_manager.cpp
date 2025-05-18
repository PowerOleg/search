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
		tx.exec("SELECT setval('" + table1 + "_id_seq', 1, false);");//tx.exec("ALTER SEQUENCE " + table1 + "_id_seq RESTART WITH 1;");
		tx.exec("SELECT setval('" + table2 + "_id_seq', 1, false);");//tx.exec("ALTER SEQUENCE " + table2 + "_id_seq RESTART WITH 1;");
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

bool Postgres_manager::Write(const std::string url, size_t postgres_count, const std::map<std::string, int>& new_words, long& word_number)
{
	bool result = false;
	if (url.length() > 255)
	{
		std::cout << "Ошибка! url длиннее 255 символов";
		return false;
	}

	std::string	document_id = "";
	//std::map<std::string, std::string> word_and_id;//key=word, value=id
	std::vector<std::string> all_words;
	std::vector<std::string> all_words_id;
	try
	{
		if (postgres_count > 1)
		{
			pqxx::work tx0{ connection };
			this->last_word_id_last_iteration = tx0.query_value<std::string>(
				"select id from words where id = (select max(id) from words);"
			);
			tx0.commit();
		}



		pqxx::work tx1{ connection };
		tx1.exec_prepared("prepared_insert_document", url);//prepared statement
		std::string word = "";
		for (const auto& word_and_quantity : new_words)
		{
			word = word_and_quantity.first;
			try
			{
				tx1.exec_prepared("prepared_insert_word", word);//prepared statement
			}
			catch (const std::exception& e)
			{
				std::cout << "error while inserting of word: " << word << std::endl;
				std::cout << e.what() << std::endl;
			}
		}
		tx1.commit();



		pqxx::work tx2{ connection };
		document_id = tx2.query_value<std::string>(
				"SELECT d.id FROM documents d where d.document = \'" + url + "\';"
			);
		tx2.commit();



		pqxx::work tx3{ connection };
		auto sql_words_table = tx3.query<std::string, std::string>(
			"SELECT w.id, w.word FROM words w;"
		);
		tx3.commit();

		for (const std::tuple<std::string, std::string>& value : sql_words_table)//refactor cause no need to create additional vectors
		{
			all_words_id.push_back(std::get<0>(value));
			all_words.push_back(std::get<1>(value));
		}


		pqxx::work tx4{ connection };
		std::cout << "Writing to the database. document_id: " << document_id << std::endl;
		for (const auto& new_word_and_quantity : new_words)
		{
			std::vector<std::string>::iterator vector_it = std::find(all_words.begin(), all_words.end(), new_word_and_quantity.first);
			size_t index = std::distance(all_words.begin(), vector_it);
			if (index >= 0)
			{
				bool compare = all_words_id.at(index).compare(this->last_word_id_last_iteration) > 0;
				//std::cout << "1: " << all_words_id.at(index) << std::endl;
				//std::cout << "2: " << this->last_word_id_last_iteration << std::endl;
				//std::cout << "result: " << compare << std::endl;
				if (compare)
				{
					tx4.exec_prepared("prepared_insert_documents_words", document_id, all_words_id.at(index), new_word_and_quantity.second);//prepared statement
				}

			}
			
		}
		tx4.commit();
		result = true;
	}
	catch (const std::exception& e)
	{
		std::cout << std::endl;
		std::cout << e.what() << std::endl << std::endl;
	}


//	//std::string document_id = std::to_string(postgres_count);

//	try
//	{
//		
//}
//catch (const std::exception& e)
//{
//	std::cout << e.what() << std::endl;
//}
	return result;
}
