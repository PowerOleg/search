#pragma once
#include "common.h"
#include <fstream>
#include <sstream> 

class File_manager
{
public:
	File_manager(const std::string filename_);

	void ReadFile();
	void WriteInFile();
	void FillConfig(
		std::string* sqlhost,
		std::string* sqlport,
		std::string* dbname,
		std::string* username,
		std::string* password,
		std::string* url,
		std::string* crowler_depth,
		std::string* http_port);
private:
	const std::string filename;
};
