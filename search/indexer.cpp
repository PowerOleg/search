#include "indexer.h"
//#include <locale>

Indexer::Indexer(const std::string& page_body)// : out{ "test.txt" }
{
	this->page_body = page_body;
	printf("Indexing...");
    std::vector<std::string> hrefs;

    GumboAttribute* hrefBase = nullptr;
    GumboOutput* output = gumbo_parse(page_body.c_str());

    std::queue<GumboNode*> qn;
    qn.push(output->root);

    while (!qn.empty())
    {
        GumboNode* node = qn.front();
        qn.pop();
        if (GUMBO_NODE_ELEMENT == node->type)
        {
            ExtractText(node);
        }
        
    }
    //out.close();
}

void Indexer::ExtractText(GumboNode* node) {
   // [4] (https://best-of-web.builder.io/library/google/gumbo-parser)
    if (node->type == GUMBO_NODE_TEXT) {
       std::string word;
       //out << node->v.text.text;//d
       //out << " ";//d
       std::string line = node->v.text.text;
       
       std::istringstream stringstream(line);
       while (!stringstream.eof())
       {
           stringstream >> word;
           
           words1.emplace_back(word);
       }
        //printf(/*"Text: %s\n", */node->v.text.text);// [4] (https://best-of-web.builder.io/library/google/gumbo-parser)
    }
    else if (node->type == GUMBO_NODE_ELEMENT)
    {
        GumboVector* children = &node->v.element.children;
        for (unsigned int i = 0; i < children->length; ++i) {
            ExtractText((GumboNode*)children->data[i]);
        }
    }
}

std::map<std::string, int> Indexer::Count(const std::vector<std::string> &words)
{
    std::map<std::string, int> count_map;
    for (size_t i = 0; i < words.size(); i++)
    {
        std::string word = words.at(i);
        if (count_map.contains(word))
        {
            int quantity = count_map.at(word);
            count_map.insert({ word , ++quantity });
        }
        else
        {
            if (word.length() > 32)
            {
                continue;
            }
            count_map.insert({ word , 1 });
        }
    }
    return count_map;
}

void Indexer::FilterSymbols(std::vector<std::string> &words)
{
    std::vector<std::string> filtered_words;
    std::regex pattern{ "[\"\\.,\\(\\[\\]':\\{\\}]+" }; //не понимает символ скобочки )
    std::smatch match;

    for (size_t i = 0; i < words.size(); i++)
    {
        std::string word = words.at(i);
        //FilterWord(words.at(i), filtered_word);
        boost::algorithm::to_lower(word);
        for (size_t i = 0; i < word.length(); i++)
        {
            std::string symbol;
            symbol.push_back(word[i]);
            if (std::regex_search(symbol, pattern))
            {
                word.replace(i, 1, " ");
            }

        }

        word.erase(std::remove(word.begin(), word.end(), ' '), word.end());
        filtered_words.push_back(word);
    }
    words = std::move(filtered_words);
}


void Indexer::FilterWord(const std::string &word, std::string &filtered_word)
{
    std::cout << word << std::endl;
    for (size_t i = 0; i < word.length(); i++)
    {
        if (std::isalpha(word[i]))
        {
            filtered_word += word[i];
        }
    }

    std::cout << filtered_word << std::endl;
}
