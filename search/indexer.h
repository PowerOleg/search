#pragma once
#include <string>
#include <vector>
#include <queue>
#include <boost/beast/core.hpp>
#include <boost/beast/http.hpp>
#include <iomanip>
#include <algorithm>
#include <sstream>
#include <map>
#include <regex>
#include <boost/algorithm/string.hpp>
#include "gumbo.h"
//#include <thread>
//#include <iostream>

class Indexer
{
public:
	Indexer(const std::string &page_body);
	std::vector<std::string> GetWords() { return this->words; }
	std::map<std::string, int> Count(const std::vector<std::string> &words);
	void FilterSymbols(std::vector<std::string>& words);

private:
	void ExtractText(GumboNode* node);
	void FilterWord(const std::string& word, std::string& filtered_word);
	std::string page_body;
	std::vector<std::string> words;
};

