#include <iostream>
#include <thread>
#include <boost/array.hpp>
#include <boost/asio.hpp>

using boost::asio::ip::tcp;

#include "TCPClient.h"

class TestClient : public net::tcp::Client
{
    using base = net::tcp::Client;

public:
    TestClient() {}

    virtual bool OnConnected() override
    {
        printf("\nConnected to the Server : %s, %d", GetServerHostname().c_str(), GetPort());
        return true;
    }

    virtual void OnConnectionError(const std::string& errorMessage)
    {
        printf("\nError Connecting to Server : %s", errorMessage.c_str());
    }

    virtual bool OnDataReceived(
        const std::shared_ptr<const std::vector<uint8_t>>& buffer, 
        std::size_t bytesRead) override
    { 
        const auto& b = *buffer;
        std::string data(b.begin(), b.begin() + bytesRead);
        printf("\nFrom Server : %s", data.c_str());
        return true; 
    }


    virtual void OnDataReceivedError(const std::string& errorMessage) override 
    { 
        printf("\nError occured while reading data : %s", errorMessage.c_str());
    }


    virtual void OnDataWritten(std::size_t bytesWritten) override
    {
        printf("\nWritten %" SCNu64 " bytes to server." CRLF, bytesWritten);
    }

    virtual void OnDisconnection() override
    {
        printf("\nDisconnected from the server.");
    }

};

int main(int argc, char* argv[])
{
    TestClient client;
    client.AsyncConnect("127.0.0.1", 8080);
    client.Wait();

    return 0;
}