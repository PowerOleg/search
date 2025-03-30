#include "indexer.h"

Indexer::Indexer(std::string text)
{
	this->page_body = text;
	printf("Indexing...");
}
