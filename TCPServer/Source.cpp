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
        TCP::Server server(8080, 2);
        server.Start();
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