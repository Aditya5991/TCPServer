#include <ctime>
#include <iostream>
#include <string>
#include "Server.h"
#include <boost/asio.hpp>

using boost::asio::ip::tcp;

std::string make_daytime_string()
{
    using namespace std; // For time_t, time and ctime;
    time_t now = time(0);
    return ctime(&now);
}

std::string read(tcp::socket& socket)
{
    boost::asio::streambuf buffer;
    boost::asio::read_until(socket, buffer, "\n");
    std::string data = boost::asio::buffer_cast<const char*>(buffer.data());
    return data;
}

std::string read_some(tcp::socket& socket)
{
    char buffer[256];
    memset(buffer, 0, sizeof(buffer));
    socket.read_some(boost::asio::buffer(buffer));
    std::string data = buffer;
    return data;
}

size_t write(tcp::socket& socket, const std::string& message)
{
    boost::system::error_code ignored_error;
    std::string msg = message + "\n";
    return boost::asio::write(socket, boost::asio::buffer(message), ignored_error);
}


void accept_handler(const boost::system::error_code& ec)
{
    if (!ec)
    {
    }
}

int tryAsync()
{
    try 
    {
        TCP::Server server(8080);
        server.Start();
    }
    catch (std::exception& e)
    {
        std::cout << e.what() << std::endl;
    }

    return 0;
}


std::vector<char> buffer(1 * 1024);

void async_read_some(tcp::socket& socket)
{
    socket.async_read_some(boost::asio::buffer(buffer.data(), buffer.size()), 
        [&](const boost::system::error_code& ec, std::size_t numBytesRead)
        {
            if (ec == boost::asio::error::eof)
            {
                std::cout << "Client Disconnected..." << std::endl;
            }

            if (!ec)
            {
                std::cout << "Num bytes read : " << numBytesRead << std::endl;
                std::cout << "Data : ";
                for (int i = 0; i < numBytesRead; ++i)
                    std::cout << buffer[i];

                async_read_some(socket);
            }
        });
}


int mainAsync()
{

    try
    {
        boost::asio::io_context context;

        boost::asio::io_context::work idleWord(context);

        std::thread contextThread = std::thread([&]() { context.run(); });

        tcp::acceptor acceptor(context, tcp::endpoint(tcp::v4(), 8080));

        tcp::socket socket(context); // client socket
        std::cout << "Waiting for new Connection..." << std::endl;
        acceptor.accept(socket);

        const std::string& ip = socket.remote_endpoint().address().to_string();
        auto port = socket.remote_endpoint().port();
        std::cout << "Client Connected : " << ip << ", " << port << std::endl;

        async_read_some(socket);

        while (1);
        //if (contextThread.joinable())
        //    contextThread.join();

    }
    catch (std::exception& e)
    {
        std::cout << e.what() << std::endl;
    }

    return EXIT_SUCCESS;
}

int main()
{
    return tryAsync();

    return mainAsync();

    try
    {
        boost::asio::io_context io_context;

        tcp::acceptor acceptor(io_context, tcp::endpoint(tcp::v4(), 8080));

        tcp::socket socket(io_context); // client socket
        std::cout << "Waiting for new Connection..." << std::endl;
        acceptor.accept(socket);

        std::string ip = socket.remote_endpoint().address().to_string();
        auto port = socket.remote_endpoint().port();
        std::cout << "Client Connected : " << ip << ", " << port << std::endl;

        std::thread readThread(
            [&]() 
            {
                while (1)
                {
                    std::string fromClient = read_some(socket);
                    std::cout << std::format("[readThread] From Client: {}", fromClient) << std::endl;
                }
            });

        std::thread writeThread(
            [&]()
            {
                while (1)
                {
                    std::cout << "Enter Message: ";
                    std::string input;
                    std::cin >> input;
                    size_t bytesWritten = write(socket, input);

                    std::cout << std::format("[writeThread] Bytes Written: {}", bytesWritten) << std::endl;
                }
            });

        readThread.join();
        writeThread.join();

        for (;;)
        {
            std::string input;
            std::cin >> input;

            //std::string message = make_daytime_string();
            std::string message = input + "\n";
            write(socket, message);
        }
    }
    catch (std::exception& e)
    {
        std::cerr << e.what() << std::endl;
    }

    return 0;
}