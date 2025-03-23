#include "crowler.h"

Crowler::Crowler(const std::string host_, const std::string target_) : host{ host_ }, target{target_}
{}

int Crowler::HttpRequest()
{
    try
    {
        boost::asio::io_context ioc;
        boost::asio::ip::tcp::resolver resolver(ioc);
        boost::asio::ip::tcp::socket socket(ioc);
        boost::asio::connect(socket, resolver.resolve(host, "80"));//magic_number
        http::request<http::string_body> req(http::verb::get, target, 11);//magic_number
        req.set(http::field::host, host);
        req.set(http::field::user_agent, BOOST_BEAST_VERSION_STRING);
        http::write(socket, req);

        {
            boost::beast::flat_buffer buffer;
            http::response<http::dynamic_body> res;
            http::read(socket, buffer, res);

            std::cout << res << std::endl;
        }
        socket.shutdown(boost::asio::ip::tcp::socket::shutdown_both);

    }
    catch (const std::exception& e)
    {
        std::cerr << "Error: " << e.what() << std::endl;
        return EXIT_FAILURE;
    }

    return 0;
}