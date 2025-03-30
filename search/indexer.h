#pragma once
#include <string>
#include <vector>

class Indexer
{
public:
	Indexer(std::string text);
	std::vector<std::string> getWords();
private:
	std::string page_body;
};

