#pragma once
#include <string>
#include <iostream>
#include <vector>
#include <queue>

#include <boost/beast/core.hpp>
#include <boost/beast/http.hpp>

#include <fstream>
#include <iomanip>
#include <iostream>
#include <algorithm>
#include <thread>
//#include <gumbo.h>
#include "gumbo.h"
#pragma execution_character_set("utf-8")

class Indexer
{
public:
	Indexer(const std::string &page_body);
	std::vector<std::string> getWords() { return this->words; }
	
private:
	void ExtractText(GumboNode* node);

	std::string page_body;
	std::vector<std::string> words;
};

