#include "Server.h"
#include <ctime>
#include <iostream>
#include <string>
#include <boost/asio.hpp>

using boost::asio::ip::tcp;

int tryAsync()
{
    try 
    {
        std::shared_ptr<TCP::Server> server = std::make_shared<TCP::Server>(8080);
        server->Start();
        auto count = server.use_count();
        return 0;
    }
    catch (std::exception& e)
    {
        std::cout << e.what() << std::endl;
    }

    return 0;
}

int main()
{
    return tryAsync();
}