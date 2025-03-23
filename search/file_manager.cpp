#include "file_manager.h"

File_manager::File_manager(const std::string filename_) : filename{filename_}
{}

void File_manager::ReadFile()
{

	
	


}

void File_manager::WriteInFile()
{

}

void File_manager::FillConfig(std::string* sqlhost, std::string* sqlport, std::string* dbname, std::string* username, std::string* password, std::string* url, std::string* crowler_depth, std::string* http_port)
{
	std::string line;
	std::ifstream in(filename);

}
