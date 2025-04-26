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
    std::cout << "regex_match: " << this->url << std::endl;
    std::chrono::milliseconds timespan(200);
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
        this->vLinks = FindLinks(page_text);

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
        this->vLinks = FindLinks(page_text);


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

//int Webpage::SimpleHttpRequest()
//{
//    try
//    {
//        boost::asio::io_context ioc;
//        boost::asio::ip::tcp::resolver resolver(ioc);
//        boost::asio::ip::tcp::socket socket(ioc);
//        boost::asio::connect(socket, resolver.resolve(host, port));
//        http::request<http::string_body> req(http::verb::get, target, version);
//        req.set(http::field::host, host);
//        req.set(http::field::user_agent, BOOST_BEAST_VERSION_STRING);
//        http::write(socket, req);
//
//        {
//            boost::beast::flat_buffer buffer;
//            http::response<http::dynamic_body> res;
//            http::read(socket, buffer, res);
//
//            std::cout << res << std::endl;
//        }
//        socket.shutdown(boost::asio::ip::tcp::socket::shutdown_both);
//
//    }
//    catch (const std::exception& e)
//    {
//        std::cerr << "Error: " << e.what() << std::endl;
//        return EXIT_FAILURE;
//    }
//
//    return 0;
//}

//int Webpage::HttpWebSocketRequest()
//{
//    try
//    {
//        // Check command line arguments.
//      /*  if (argc != 4)
//        {
//            std::cerr <<
//                "Usage: websocket-client-sync <host> <port> <text>\n" <<
//                "Example:\n" <<
//                "    websocket-client-sync echo.websocket.org 80 \"Hello, world!\"\n";
//            return EXIT_FAILURE;
//        }*/
//        //std::string host = argv[1];
//        //auto const  port = argv[2];
//        auto const  text = "";
//
//        // The io_context is required for all I/O
//        net::io_context ioc;
//
//        // These objects perform our I/O
//        tcp::resolver resolver{ ioc };
//        websocket::stream<tcp::socket> ws{ ioc };
//
//        // Look up the domain name
//        auto const results = resolver.resolve(host, port);
//
//        // Make the connection on the IP address we get from a lookup
//        auto ep = net::connect(ws.next_layer(), results);
//
//        // Update the host_ string. This will provide the value of the
//        // Host HTTP header during the WebSocket handshake.
//        // See https://tools.ietf.org/html/rfc7230#section-5.4
//        host += ":" +std::to_string(ep.port());//uri - host[":" port]
//
//        // Set a decorator to change the User-Agent of the handshake
//        ws.set_option(websocket::stream_base::decorator(
//            [](websocket::request_type& req)
//            {
//                req.set(http::field::user_agent,
//                    std::string(BOOST_BEAST_VERSION_STRING) +
//                    " websocket-client-coro");
//            }));
//
//        // Perform the websocket handshake
//        ws.handshake(host, "/");
//
//        // Send the message
//        ws.write(net::buffer(std::string(text)));
//
//        // This buffer will hold the incoming message
//        beast::flat_buffer buffer;
//
//        // Read a message into our buffer
//        ws.read(buffer);
//
//        // Close the WebSocket connection
//        ws.close(websocket::close_code::normal);
//
//        // If we get here then the connection is closed gracefully
//
//        // The make_printable() function helps print a ConstBufferSequence
//        std::cout << beast::make_printable(buffer.data()) << std::endl;
//    }
//    catch (std::exception const& e)
//    {
//        std::cerr << "Error: " << e.what() << std::endl;
//        return EXIT_FAILURE;
//    }
//    return EXIT_SUCCESS;
//}
