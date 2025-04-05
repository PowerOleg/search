#include "indexer.h"
//#include <locale>

Indexer::Indexer(const std::string& page_body) : out{ "test.txt" }
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
    out.close();
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
           
           words.emplace_back(word);
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
