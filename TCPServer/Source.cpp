#include "Server.h"
#include "Client.h"
#include <ctime>
#include <iostream>
#include <string>
#include <boost/asio.hpp>

using boost::asio::ip::tcp;


class TestServer : public TCP::Server
{
public:
    using base = TCP::Server;

    TestServer(int port, uint32_t maxClientsAllowed = -1)
        : base(port, maxClientsAllowed)
    {
    }

    virtual ~TestServer() 
    {
    }
    
    virtual bool OnClientConnected(TCP::Client* newClient) override
    {
        if (newClient == nullptr)
            return false;

        std::string newConnectionMessage 
            = std::format("New Client Connected : {}" CRLF, newClient->GetInfoString());
        std::vector<uint8_t> buffer(newConnectionMessage.begin(), newConnectionMessage.end());

        base::MessageAllClients(buffer, newClient);

        return true;
    }

    virtual void OnDataReceived(TCP::Client* client) override
    {
        const auto& buffer = client->GetReadBuffer();
        std::size_t bytesRead = client->GetBytesRead();
        const auto& clientInfo = client->GetInfoString();

        std::string data(buffer.begin(), buffer.begin() + bytesRead);
        printf("\nFrom %s : %s", clientInfo.c_str(), data.c_str());
    }

};


int main()
{
    TestServer server(8080);
    server.Start();
    server.Wait();

    return 0;
}