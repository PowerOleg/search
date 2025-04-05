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
#include <sstream>

#include "gumbo.h"

class Indexer
{
public:
	Indexer(const std::string &page_body);
	std::vector<std::string> getWords() { return this->words; }
	std::ofstream out;
private:
	void ExtractText(GumboNode* node);

	std::string page_body;
	std::vector<std::string> words;
};

