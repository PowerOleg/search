#pragma once

#include <boost/beast/core.hpp>
#include <boost/beast/http.hpp>
#include <boost/beast/version.hpp>
#include <boost/asio/connect.hpp>
#include <boost/asio/ip/tcp.hpp>
#include <cstdlib>
#include "common.h"

namespace beast = boost::beast;     // from <boost/beast.hpp>
namespace http = beast::http;       // from <boost/beast/http.hpp>
namespace net = boost::asio;        // from <boost/asio.hpp>
using tcp = net::ip::tcp;           // from <boost/asio/ip/tcp.hpp>


class Crowler
{
public:
	Crowler(const std::string host_, const std::string target_);
    int HttpRequest()
    {
        try
        {
        //const std::string host = "api.openweathermap.org";
        //const std::string target = "/data/2.5/weather?q=Moscow,ru,pt&APPID=cdecb72a7903eb3bb1964c39615f1764";//" / echo ? input = test";
        //const std::string host = "httpbin.org";
        //const std::string target = "/get";
    
        // I/O контекст, необходимый для всех I/O операций
        boost::asio::io_context ioc;

        // Resolver для определения endpoint'ов
        boost::asio::ip::tcp::resolver resolver(ioc);
        // Tcp сокет, использующейся для соединения
        boost::asio::ip::tcp::socket socket(ioc);

        // Резолвим адрес и устанавливаем соединение
        boost::asio::connect(socket, resolver.resolve(host, "80"));

        // Дальше необходимо создать HTTP GET реквест с указанием таргета
        http::request<http::string_body> req(http::verb::get, target, 11);
        // Задаём поля HTTP заголовка
        req.set(http::field::host, host);
        req.set(http::field::user_agent, BOOST_BEAST_VERSION_STRING);

        // Отправляем реквест через приконекченный сокет
        http::write(socket, req);

        // Часть, отвечающая за чтение респонса
        {
            boost::beast::flat_buffer buffer;
            http::response<http::dynamic_body> res;
            http::read(socket, buffer, res);

            std::cout << res << std::endl;
        }
        // Закрываем соединение
        socket.shutdown(boost::asio::ip::tcp::socket::shutdown_both);


        }
        catch (const std::exception& e)
        {
            std::cerr << "Error: " << e.what() << std::endl;
            return EXIT_FAILURE;
        }

        return 0;
    }

	/*int Test()
	{
		try
		{
            // The io_context is required for all I/O
            net::io_context ioc;

            // These objects perform our I/O
            tcp::resolver resolver(ioc);
            beast::tcp_stream stream(ioc);

            // Look up the domain name
            auto const results = resolver.resolve(host, port);

            // Make the connection on the IP address we get from a lookup
            stream.connect(results);

            // Set up an HTTP GET request message
            http::request<http::string_body> req{ http::verb::get, target, version };
            req.set(http::field::host, host);
            req.set(http::field::user_agent, BOOST_BEAST_VERSION_STRING);

            // Send the HTTP request to the remote host
            http::write(stream, req);

            // This buffer is used for reading and must be persisted
            beast::flat_buffer buffer;

            // Declare a container to hold the response
            http::response<http::dynamic_body> res;

            // Receive the HTTP response
            http::read(stream, buffer, res);

            // Write the message to standard out
            std::cout << res << std::endl;

            // Gracefully close the socket
            beast::error_code ec;
            stream.socket().shutdown(tcp::socket::shutdown_both, ec);

            // not_connected happens sometimes
            // so don't bother reporting it.
            //
            if (ec && ec != beast::errc::not_connected)
                throw beast::system_error{ ec };

            // If we get here then the connection is closed gracefully
            return 0;
        }
        catch (std::exception const& e)
        {
            std::cerr << "Error: " << e.what() << std::endl;
            return EXIT_FAILURE;
        }
		


	}*/

private:
	const std::string host;
    const std::string target;
	const std::string port;
	const int version = 11;
};

