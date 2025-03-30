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
//#include <gumbo.h>
#include "gumbo.h"

class Indexer
{
public:
	Indexer(const std::string text);
	std::vector<std::string> getWords() { return this->words; }
	void ExtractText(GumboNode* node);
private:
	std::string page_body;
	std::vector<std::string> words;
};

