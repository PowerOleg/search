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
#include <map>
#include <regex>
#include <boost/algorithm/string.hpp>
#include "gumbo.h"


class Indexer
{
public:
	Indexer(const std::string &page_body);
	std::vector<std::string> getWords() { return this->words; }
	//std::ofstream out;
	std::map<std::string, int> Count(const std::vector<std::string> &words);
	void FilterSymbols(std::vector<std::string>& words);

private:
	void ExtractText(GumboNode* node);
	void FilterWord(const std::string& word, std::string& filtered_word);
	std::string page_body;
	std::vector<std::string> words;
};

