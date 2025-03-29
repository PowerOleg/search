#include "crawler.h"

Crawler::Crawler(std::string host_, const std::string target_) : host{ host_ }, target{target_}
{}

int Crawler::SimpleHttpRequest()
{
    try
    {
        boost::asio::io_context ioc;
        boost::asio::ip::tcp::resolver resolver(ioc);
        boost::asio::ip::tcp::socket socket(ioc);
        boost::asio::connect(socket, resolver.resolve(host, port));
        http::request<http::string_body> req(http::verb::get, target, version);
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

int Crawler::HttpWebSocketRequest()
{
    try
    {
        // Check command line arguments.
      /*  if (argc != 4)
        {
            std::cerr <<
                "Usage: websocket-client-sync <host> <port> <text>\n" <<
                "Example:\n" <<
                "    websocket-client-sync echo.websocket.org 80 \"Hello, world!\"\n";
            return EXIT_FAILURE;
        }*/
        //std::string host = argv[1];
        //auto const  port = argv[2];
        auto const  text = "";

        // The io_context is required for all I/O
        net::io_context ioc;

        // These objects perform our I/O
        tcp::resolver resolver{ ioc };
        websocket::stream<tcp::socket> ws{ ioc };

        // Look up the domain name
        auto const results = resolver.resolve(host, port);

        // Make the connection on the IP address we get from a lookup
        auto ep = net::connect(ws.next_layer(), results);

        // Update the host_ string. This will provide the value of the
        // Host HTTP header during the WebSocket handshake.
        // See https://tools.ietf.org/html/rfc7230#section-5.4
        host += ":" +std::to_string(ep.port());//uri - host[":" port]

        // Set a decorator to change the User-Agent of the handshake
        ws.set_option(websocket::stream_base::decorator(
            [](websocket::request_type& req)
            {
                req.set(http::field::user_agent,
                    std::string(BOOST_BEAST_VERSION_STRING) +
                    " websocket-client-coro");
            }));

        // Perform the websocket handshake
        ws.handshake(host, "/");

        // Send the message
        ws.write(net::buffer(std::string(text)));

        // This buffer will hold the incoming message
        beast::flat_buffer buffer;

        // Read a message into our buffer
        ws.read(buffer);

        // Close the WebSocket connection
        ws.close(websocket::close_code::normal);

        // If we get here then the connection is closed gracefully

        // The make_printable() function helps print a ConstBufferSequence
        std::cout << beast::make_printable(buffer.data()) << std::endl;
    }
    catch (std::exception const& e)
    {
        std::cerr << "Error: " << e.what() << std::endl;
        return EXIT_FAILURE;
    }
    return EXIT_SUCCESS;
}
