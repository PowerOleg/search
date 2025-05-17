#include "webpage.h"

Webpage::Webpage(boost::asio::io_context& ioc_, std::string url_) : ioc{ ioc_ },  url(url_)
{
    //this->url = "https://mail.ru/";
    //boost::asio::io_context ioc_;
    //this->ioc = &ioc_;
}



void Webpage::LoadPage()
{
    std::smatch match;
    //std::cout << "regex_match: " << this->url << std::endl;
    std::chrono::milliseconds timespan(100);
    std::this_thread::sleep_for(timespan);
    if (std::regex_match(this->url, match, regex_pattern))
    {
        if (match[1].str() == "http")
        {
            LoadHttp(match);
        }
        else
        {
            LoadHttps(match);
        }
    }
    else
    {
        {
            std::lock_guard<std::mutex> lg{ mtx };
            std::cerr << "Error!!!LoadPage() std::regex_match() failed: " + url << "\nThis is not an url\n";
        }
    }
}

bool Webpage::IsValid()
{
    return is_valid_page;
}

void Webpage::SetValid()
{
    this->is_valid_page = true;
}

std::vector<std::string> Webpage::LoadHttp(const std::smatch& match)
{
    std::vector<std::string> vLinks;
    try {
        const std::string target = (match[3].length() == 0 ? "/" : match[3].str());
        int version = 11;

        tcp::socket socket{ ioc };
        tcp::resolver resolver{ ioc };
        std::string match_element2 = match[2].str();
        const auto results = resolver.resolve(match_element2, "80");
        boost::asio::connect(socket, results.begin(), results.end());
        http::request<http::string_body> req{ http::verb::get, target, version };
        req.set(http::field::host, match_element2);
        req.set(http::field::user_agent, BOOST_BEAST_VERSION_STRING);
        http::write(socket, req);
        boost::beast::flat_buffer buffer;
        http::response<http::dynamic_body> res;
        http::read(socket, buffer, res);

        this->page_text = boost::beast::buffers_to_string(res.body().data());
        std::vector<std::string> abs_links;
        std::vector<std::string> links = FindLinks(page_text);
        AbsLinks(links, abs_links);
        this->page_links = std::move(abs_links);
        std::cout << "ProcessingPage73: " << url << " has " << this->page_links.size() << " links" << std::endl;

        boost::system::error_code ec;
        socket.shutdown(tcp::socket::shutdown_both, ec);
        if (ec && ec != boost::system::errc::not_connected)
            throw boost::system::system_error{ ec };

    }
    catch (const std::exception& e)
    {
        {
            std::lock_guard<std::mutex> lg{ mtx };
            std::cerr << "loadHttp(): " << e.what() << std::endl;
        }
    }
    return vLinks;
}

std::vector<std::string> Webpage::LoadHttps(std::smatch const& match)
{
    std::vector<std::string> vLinks;
    try {

        std::string const host = match[2];
        std::string const target = (match[3].length() == 0 ? "/" : match[3].str());
        int version = 11;

        boost::asio::ssl::context ctx{ boost::asio::ssl::context::sslv23_client };
        //boost::asio::ssl::context ctx{ net::ssl::verify_none };
        ctx.set_verify_mode(net::ssl::verify_none);
        ctx.set_default_verify_paths();
        //load_root_certificates(ctx);
        boost::asio::ssl::stream<tcp::socket> stream{ ioc, ctx };
        if (!SSL_set_tlsext_host_name(stream.native_handle(), host.c_str()))
        {
            boost::system::error_code ec{ static_cast<int>(::ERR_get_error()), boost::asio::error::get_ssl_category() };
            throw boost::system::system_error{ ec };
        }
        tcp::resolver resolver{ ioc };
        auto const results = resolver.resolve(host, "443");
        boost::asio::connect(stream.next_layer(), results.begin(), results.end());
        stream.handshake(boost::asio::ssl::stream_base::client);
        http::request<http::string_body> req{ http::verb::get, target, version };
        req.set(http::field::host, host);
        req.set(http::field::user_agent, BOOST_BEAST_VERSION_STRING);
        http::write(stream, req);
        boost::beast::flat_buffer buffer;
        http::response<http::dynamic_body> res;
        http::read(stream, buffer, res);

        this->page_text = boost::beast::buffers_to_string(res.body().data());
        std::vector<std::string> abs_links;
        std::vector<std::string> links_temp = FindLinks(page_text);
        AbsLinks(std::move(links_temp), abs_links);
        this->page_links = std::move(abs_links);
        std::cout << "ProcessingPage128: " << url << " has " << this->page_links.size() << " links" << std::endl;

        boost::system::error_code ec;
        stream.shutdown(ec);
        if (ec == boost::asio::error::eof)
        {
            // Rationale:
            // http://stackoverflow.com/questions/25587403/boost-asio-ssl-async-shutdown-always-finishes-with-an-error
            ec.assign(0, ec.category());
        }
        if (ec)
            throw boost::system::system_error{ ec };
    }
    catch (std::exception const& e)
    {
        {
            std::lock_guard<std::mutex> lg{ mtx };
            std::cerr << "loadHttps(): " << e.what() << std::endl;
        }
    }
    return vLinks;
}

std::vector<std::string> Webpage::FindLinks(std::string const& sBody)
{
    std::vector<std::string> vLinks;

    GumboAttribute* hrefBase = nullptr;
    GumboOutput* output = gumbo_parse(sBody.c_str());

    std::queue<GumboNode*> qn;
    qn.push(output->root);

    while (!qn.empty())
    {
        GumboNode* node = qn.front();
        qn.pop();
        if (GUMBO_NODE_ELEMENT == node->type)
        {
            GumboAttribute* href = nullptr;
            if (node->v.element.tag == GUMBO_TAG_A && (href = gumbo_get_attribute(&node->v.element.attributes, "href")))
            {
                std::string sLnk{ href->value };
                if (!sLnk.empty())
                {
                    vLinks.emplace_back(href->value);
                }
            }
            else if (node->v.element.tag == GUMBO_TAG_BASE && (hrefBase = gumbo_get_attribute(&node->v.element.attributes, "href")))
            {
            }
            GumboVector* children = &node->v.element.children;
            for (unsigned int i = 0; i < children->length; ++i)
            {
                qn.push(static_cast<GumboNode*>(children->data[i]));
            }
        }
    }

    if (hrefBase) // с учётом тега <base>
    {
        std::string sBase = hrefBase->value;
        {
            //std::lock_guard<std::mutex> lg{ mtx };
            //std::cout << "<base> found: " << sBase << "\n\n";
        }
        if (sBase.back() == '/')
        {
            sBase.pop_back();
        }
        for (auto& sLnk : vLinks)
        {
            if (std::regex_match(sLnk, std::regex{ "(?:[^/]+/)+[^/]+" }) || std::regex_match(sLnk, std::regex{ "[^/#?]+" })) // относительно дочерней или текущей директории
            {
                sLnk = sBase + '/' + sLnk;
            }
            else if (sLnk.find("../") == 0) // относительно родительской директории
            {
                int ind = std::string::npos;
                int cnt = (sLnk.rfind("../") + 3) / 3;
                for (int i = 0; i < cnt + 1; ++i)
                {
                    ind = sBase.rfind('/', ind - 1);
                }
                sLnk = std::string{ sBase.begin(), sBase.begin() + ind + 1 } + std::string{ sLnk.begin() + cnt * 3, sLnk.end() };
            }
            sLnk;
        }
    }

    gumbo_destroy_output(&kGumboDefaultOptions, output);
    return vLinks;
}



void Webpage::AbsLinks(const std::vector<std::string> &init_links, std::vector<std::string> &abs_links)
{
    std::smatch mr;
    
    int start_search_index = this->url.find("//");
    int host_end_index = this->url.find("/", start_search_index + 2);
    std::string hostname = this->url;
    hostname[host_end_index] = '\0';
    std::regex regex_pattern{ "^(?:(https?)://)([^/]+)(/.*)?" };
    bool found_host = false;
    for (int i = 0; i < init_links.size(); ++i)
    {
        std::string link = init_links.at(i);
        if (link == "" || link.find("/") == -1)
        {
            std::cout << "239Wrong link to make absolute: " << link << std::endl;
            continue;
        }

        if (link.find("//") == 0) // относительно протокола
        {
            std::regex_search(link, mr, std::regex{ "^[^/]+" });
            link = hostname + link;//sUri = mr.str() + sUri;

        }
        else if (link.find('/') == 0) // относительно имени хоста
        {
            std::regex_search(link, mr, std::regex{ "^[^/]+//[^/]+" });
            link = hostname + link;//sUri = mr.str() + sUri;

        }
        else if (link.find("../") == 0) // относительно родительской директории
        {
            int ind = std::string::npos;
            int cnt = (link.rfind("../") + 3) / 3;
            for (int i = 0; i < cnt + 1; ++i)
            {
                ind = link.rfind('/', ind - 1);
            }
            link = std::string{ link.begin(), link.begin() + ind + 1 } + std::string{ link.begin() + cnt * 3, link.end() };

        }
        else if (std::regex_match(link, std::regex{ "(?:[^/]+/)+[^/]+" }) || std::regex_match(link, std::regex{ "[^/#?]+" })) // относительно дочерней директории или просто имя файла
        {
            std::cout << "285: " << link << std::endl;
            int ind = link.rfind('/');
            link = std::string{ link.begin(), link.begin() + ind + 1 } + link;
            std::cout << "288" << std::endl;
        }

        if (std::regex_search(link, regex_pattern))
        {
            abs_links.push_back(link);
        }

    }
}

//void Webpage::IsCorrectHost(std::string& link, std::string& host, bool& found_host)
//{
//    if (this->url.find(link) != std::string::npos)
//    {
//        host = link;
//        found_host = true;
//    }
//}
