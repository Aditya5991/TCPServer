#include "Server.h"
#include "Client.h"
#include "IOBuffer.h"
#include <ctime>
#include <iostream>
#include <string>
#include <boost/asio.hpp>

using boost::asio::ip::tcp;

class TestServer : public net::tcp::Server
{
public:
    using base = net::tcp::Server;

    TestServer(int port, uint32_t maxClientsAllowed = -1)
        : base(port, maxClientsAllowed)
    {
    }

    virtual ~TestServer() 
    {
    }
    
    virtual bool OnClientConnected(net::tcp::ClientID newClientID) override
    {
        const net::tcp::Client* newClient = base::GetClient(newClientID);

        std::string newConnectionMessage 
            = std::format("New Client Connected : {}" CRLF, newClient->GetInfoString());
        std::vector<uint8_t> buffer(newConnectionMessage.begin(), newConnectionMessage.end());

        std::string IDMessage = std::format("Your ID is : {}" CRLF, newClientID);
        std::vector<uint8_t> IDMessageBuffer(IDMessage.begin(), IDMessage.end());

        base::MessageClient(newClientID, IDMessageBuffer);

        // don't send message to the newly connected client.
        base::MessageAllClients(buffer, buffer.size(), newClientID);

        return true;
    }

    virtual void OnDataReceived(net::tcp::ClientID ID) override
    {
        const net::tcp::Client* client = base::GetClient(ID);

        const auto& buffer = client->GetReadBuffer();
        std::size_t bytesRead = client->GetBytesRead();
        const auto& clientInfo = client->GetInfoString();

        std::string message(buffer.begin(), buffer.begin() + bytesRead);
        if (message == "test\r\n")
        {
            std::string toSendMessage = std::format("test message from : {}" CRLF, clientInfo);
            std::vector<uint8_t> toSendBuffer(toSendMessage.begin(), toSendMessage.end());

            base::MessageAllClients(toSendBuffer, toSendBuffer.size(), ID);
        }

        std::string data(buffer.begin(), buffer.begin() + bytesRead);
        printf("\nFrom %s : %s", clientInfo.c_str(), data.c_str());
    }

    virtual void OnClientDisconnected(net::tcp::ClientID ID) override
    {
        const net::tcp::Client* client = base::GetClient(ID);

        const auto& clientInfo = client->GetInfoString();
        std::string message = std::format("{} Disconnected..." CRLF, clientInfo);
        std::vector<uint8_t> buffer(message.begin(), message.end());

        base::MessageAllClients(buffer, buffer.size(), ID);

        base::OnClientDisconnected(ID);
    }

};


int main()
{
    TestServer server(8080);
    server.Start();
    server.Wait();

    return 0;
}