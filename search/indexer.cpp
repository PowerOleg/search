#include "indexer.h"

Indexer::Indexer(const std::string page_body)
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

            /*GumboNode* title_text = static_cast<GumboNode*>(node->v.element.children.data[0]);
            if (title_text->type == GUMBO_NODE_TEXT)
            {
                auto text1 = title_text->v.text.text;
                std::cout << text1 << std::endl;
            }*/


            //const char* text = node->v.text.text;
            //std::cout << text << std::endl;


            

            /*GumboAttribute* href = nullptr;
            if (node->v.element.tag == GUMBO_TAG_A && (href = gumbo_get_attribute(&node->v.element.attributes, "href")))
            {
                std::string sLnk{ href->value };
                if (!sLnk.empty())
                {
                    hrefs.emplace_back(href->value);
                }
            }
            else if (node->v.element.tag == GUMBO_TAG_BASE && (hrefBase = gumbo_get_attribute(&node->v.element.attributes, "href")))
            {
            }
            GumboVector* children = &node->v.element.children;
            for (unsigned int i = 0; i < children->length; ++i)
            {
                qn.push(static_cast<GumboNode*>(children->data[i]));
            }*/
            
        }
        
    }
    
}

void Indexer::ExtractText(GumboNode* node) {
   // [4] (https://best-of-web.builder.io/library/google/gumbo-parser)
    if (node->type == GUMBO_NODE_TEXT) {
        printf("Text: %s\n", node->v.text.text);// [4] (https://best-of-web.builder.io/library/google/gumbo-parser)
    }
    else if (node->type == GUMBO_NODE_ELEMENT)
    {
        GumboVector* children = &node->v.element.children;
        for (unsigned int i = 0; i < children->length; ++i) {
            ExtractText((GumboNode*)children->data[i]);
        }
    }
}
