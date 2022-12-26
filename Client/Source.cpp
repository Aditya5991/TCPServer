#include <iostream>
#include <thread>
#include <boost/array.hpp>
#include <boost/asio.hpp>

using boost::asio::ip::tcp;


size_t write(tcp::socket& socket, const std::string& msg)
{
    boost::system::error_code ec;
    std::string message = msg + "\n";
    return boost::asio::write(socket, boost::asio::buffer(message), ec);
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
    try
    {
        memset(buffer, 0, sizeof(buffer));

        boost::system::error_code ec;
        socket.read_some(boost::asio::buffer(buffer), ec);
        if (ec == boost::asio::error::eof)
        {
            printf("\nDisconnected From Server...");
            return "";
        }
    }
    catch (std::exception& e)
    {
        printf("\n%s", e.what());
    }

    std::string data = buffer;
    return data;
}

/*
    2 threads:
    -   sending request to server
    -   parsing the response
*/

int main(int argc, char* argv[])
{
    try
    {

        std::string host = "localhost";

        boost::asio::io_context io_context;

        tcp::resolver resolver(io_context);
        tcp::resolver::results_type endpoints =
            resolver.resolve(host, "8080");


        tcp::socket socket(io_context);
        auto endpoint = boost::asio::connect(socket, endpoints);

        auto serverAddress = endpoint.address().to_string();
        auto serverPort = endpoint.port();
        std::cout << "Connected to Server : " << serverAddress << ", " << serverPort << std::endl;

        std::thread readThread(
            [&]() {

                while (1)
                {
                    std::string fromServer = read_some(socket);
                    if (fromServer == "")
                        break;

                    std::cout << std::format("[readThread] From Server: {}", fromServer) << std::endl;
                }

            });

        std::thread writeThread(
            [&]() {

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
    }
    catch (std::exception& e)
    {
        std::cerr << e.what() << std::endl;
    }

    return 0;
}