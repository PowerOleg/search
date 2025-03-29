#pragma once
#include "C:/cpp/boost_1_87Bin/libs/beast/example/common/root_certificates.hpp" //прописать свой путь
#include <boost/beast/core.hpp>
#include <boost/beast/http.hpp>
#include <boost/beast/version.hpp>
#include <boost/asio/connect.hpp>
#include <boost/asio/ip/tcp.hpp>
#include <boost/asio/ssl/error.hpp>
#include <boost/asio/ssl/stream.hpp>
#include <boost/asio/thread_pool.hpp>
#include <cstdlib>
#include <iostream>
#include <string>
#include <fstream>
#include <regex>
#include <queue>
#include <vector>
#include <thread>
#include <mutex>
#include <functional>
#include <iomanip>
#include <unordered_set>
#include "gumbo.h"

using tcp = boost::asio::ip::tcp;
namespace ssl = boost::asio::ssl;
namespace http = boost::beast::http;

class Webloader
{
public:
    Webloader(boost::asio::io_context& ioc_) : ioc(ioc_) {}

    std::vector<std::string> findLinks(std::string const& sBody)
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

    void load(std::string const& sUri, std::vector<std::string>& vres)
    {
        std::smatch mr;
        if (std::regex_match(sUri, mr, rUri))
        {
            if (mr[1].str() == "http")
            {
                vres = loadHttp(mr);
            }
            else
            {
                vres = loadHttps(mr);
            }
        }
        else
        {
            {
                std::lock_guard<std::mutex> lg{ mtx };
                std::cerr << "load() std::regex_match() failed: " + sUri << "\n\n";
            }
        }
    }

    std::vector<std::string> loadHttp(const std::smatch& mr)
    {
        std::vector<std::string> vLinks;
        try {
            std::string const target = (mr[3].length() == 0 ? "/" : mr[3].str());
            int version = 11; // или 10 для http 1.0

            tcp::socket socket{ ioc };
            tcp::resolver resolver{ ioc };
            std::string mr2 = mr[2];
            auto const results = resolver.resolve(mr2, "80");
            boost::asio::connect(socket, results.begin(), results.end());
            http::request<http::string_body> req{ http::verb::get, target, version };
            req.set(http::field::host, mr2);
            req.set(http::field::user_agent, BOOST_BEAST_VERSION_STRING);
            http::write(socket, req);
            boost::beast::flat_buffer buffer;
            http::response<http::dynamic_body> res;
            http::read(socket, buffer, res);

            std::string sBody = boost::beast::buffers_to_string(res.body().data());
            vLinks = findLinks(sBody);

            boost::system::error_code ec;
            socket.shutdown(tcp::socket::shutdown_both, ec);
            if (ec && ec != boost::system::errc::not_connected)
                throw boost::system::system_error{ ec };

        }
        catch (std::exception const& e)
        {
            {
                std::lock_guard<std::mutex> lg{ mtx };
                std::cerr << "loadHttp(): " << e.what() << std::endl;
            }
        }
        return vLinks;
    }

    std::vector<std::string> loadHttps(std::smatch const& mr)
    {
        std::vector<std::string> vLinks;
        try {

            std::string const host = mr[2];
            std::string const target = (mr[3].length() == 0 ? "/" : mr[3].str());
            int version = 11;

            ssl::context ctx{ ssl::context::sslv23_client };
            load_root_certificates(ctx);
            ssl::stream<tcp::socket> stream{ ioc, ctx };
            if (!SSL_set_tlsext_host_name(stream.native_handle(), host.c_str()))
            {
                boost::system::error_code ec{ static_cast<int>(::ERR_get_error()), boost::asio::error::get_ssl_category() };
                throw boost::system::system_error{ ec };
            }
            tcp::resolver resolver{ ioc };
            auto const results = resolver.resolve(host, "443");
            boost::asio::connect(stream.next_layer(), results.begin(), results.end());
            stream.handshake(ssl::stream_base::client);
            http::request<http::string_body> req{ http::verb::get, target, version };
            req.set(http::field::host, host);
            req.set(http::field::user_agent, BOOST_BEAST_VERSION_STRING);
            http::write(stream, req);
            boost::beast::flat_buffer buffer;
            http::response<http::dynamic_body> res;
            http::read(stream, buffer, res);

            std::string sBody = boost::beast::buffers_to_string(res.body().data());
            vLinks = findLinks(sBody);

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

    Webloader(Webloader const&) = delete;
    Webloader& operator=(Webloader const&) = delete;

private:
    std::regex rUri{ "^(?:(https?)://)([^/]+)(/.*)?" };
    std::mutex mtx;
    boost::asio::io_context& ioc;
};

// копирует найденные ссылки в ustUsed и удаляет дубликаты и ссылки на фрагменты
void removeDuplicatesFragments(std::vector<std::vector<std::string>>& vres, std::unordered_set<std::string>& ustUsed)
{
    for (auto& vLnk : vres)
    {
        for (int i = 0; i < vLnk.size(); ++i)
        {
            if (vLnk.at(i).at(0) == '#' || !ustUsed.emplace(vLnk.at(i)).second)
            {
                vLnk.erase(vLnk.begin() + i);
                --i;
            }
        }
    }
}

// исправляет некоторые относительные ссылки в абсолютные
void absLinks(std::vector<std::string> const& vUri, std::vector<std::vector<std::string>>& vres)
{
    std::smatch mr;
    for (int i = 0; i < vUri.size(); ++i)
    {
        std::string sUri = vUri.at(i);
        for (auto& sLnk : vres.at(i))
        {
            if (sLnk.find("//") == 0) // относительно протокола
            {
                std::regex_search(sUri, mr, std::regex{ "^[^/]+" });
                sLnk = mr.str() + sLnk;
            }
            else if (sLnk.find('/') == 0) // относительно имени хоста
            {
                std::regex_search(sUri, mr, std::regex{ "^[^/]+//[^/]+" });
                sLnk = mr.str() + sLnk;
            }
            else if (sLnk.find("../") == 0) // относительно родительской директории
            {
                int ind = std::string::npos;
                int cnt = (sLnk.rfind("../") + 3) / 3;
                for (int i = 0; i < cnt + 1; ++i)
                {
                    ind = sUri.rfind('/', ind - 1);
                }
                sLnk = std::string{ sUri.begin(), sUri.begin() + ind + 1 } + std::string{ sLnk.begin() + cnt * 3, sLnk.end() };
            }
            else if (std::regex_match(sLnk, std::regex{ "(?:[^/]+/)+[^/]+" }) || std::regex_match(sLnk, std::regex{ "[^/#?]+" })) // относительно дочерней директории или просто имя файла
            {
                int ind = sUri.rfind('/');
                sLnk = std::string{ sUri.begin(), sUri.begin() + ind + 1 } + sLnk;
            }

            //std::cout << sLnk << std::endl;
        }
    }
}



