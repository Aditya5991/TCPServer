#include "Server.h"
#include "ClientHandler.h"
#include "IOBuffer.h"
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
        const net::tcp::ClientHandler* newClientHandle = base::GetClient(newClientID);

        printf("\nClient Connected : %s", newClientHandle->GetInfoString().c_str());

        std::string newConnectionMessage 
            = std::format("New Client Connected : {}" CRLF, newClientHandle->GetInfoString());
        std::vector<uint8_t> buffer(newConnectionMessage.begin(), newConnectionMessage.end());

        std::string IDMessage = std::format("Your ID is : {}" CRLF, newClientID);
        std::vector<uint8_t> IDMessageBuffer(IDMessage.begin(), IDMessage.end());

        base::MessageClient(newClientID, IDMessageBuffer, IDMessageBuffer.size());

        // don't send message to the newly connected client.
        base::MessageAllClients(buffer, buffer.size(), newClientID);

        return true;
    }

    virtual void OnDataReceived(net::tcp::ClientID ID) override
    {
        const net::tcp::ClientHandler* clientHandle = base::GetClient(ID);

        const auto& buffer = clientHandle->GetReadBuffer();
        std::size_t bytesRead = clientHandle->GetBytesRead();
        const auto& clientInfo = clientHandle->GetInfoString();

        std::string data(buffer.begin(), buffer.begin() + bytesRead);
        printf("\nFrom %s : %s", clientInfo.c_str(), data.c_str());
    }

    virtual void OnClientDisconnected(net::tcp::ClientID ID) override
    {
        const net::tcp::ClientHandler* clientHandle = base::GetClient(ID);

        const auto& clientInfo = clientHandle->GetInfoString();

        printf("\n%s Disconnected...", clientInfo.c_str());

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