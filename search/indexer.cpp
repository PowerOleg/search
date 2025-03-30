#include "indexer.h"

Indexer::Indexer(std::string text)
{
	this->page_body = text;
	printf("Indexing...");
}

std::vector<std::string> Indexer::getWords()
{
	std::vector<std::string> words;
	return words;
}
