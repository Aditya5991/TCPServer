#include "Server.h"
#include "Client.h"
#include <iostream>
#include <vector>
#include <array>
#include <algorithm>
#include <boost/asio.hpp>
#include <boost/bind/bind.hpp>

using boost::asio::ip::tcp;

#define CRLF "\r\n"

BEGIN_NAMESPACE_TCP

// public
Server::Server(int port)
    : m_Port(port)
    , m_IOContext()
    , m_Acceptor(IOContext(), tcp::endpoint(tcp::v4(), port))
    , m_NewClientID(1)
{
}

// public
Server::~Server()
{
    Stop();
}

// public
bool Server::Start()
{
    // order is important, as the context needs some work to do, else it will end the context thread.
    WaitToAcceptNewConnection();
    m_ContextThread = std::thread([this]()
        {
                IOContext().run(); 
        });

    return false;
}

// public
bool Server::Stop()
{
    if(m_ContextThread.joinable())
        m_ContextThread.join();

    IOContext().stop();

    return false;
}

// public
void Server::OnDataReceived(Client* client, std::size_t bytesRead)
{
    const auto& buffer = client->GetReadBuffer();
    const auto& clientInfo = client->GetInfoString();

    std::string data(buffer.begin(), buffer.begin() + bytesRead);
    printf("\nFrom %s : %s", clientInfo.c_str(), data.c_str());

    if (data == "time" CRLF)
    {
        std::string temp = "requested time" CRLF;
        std::vector<uint8_t> toWrite(temp.begin(), temp.end());
        Write(client, toWrite);
    }

}

// public
void Server::OnDataReceivedError(Client* client, const boost::system::error_code& ec)
{
    printf("Error reading from Client : %s", ec.message().c_str());
    OnDisconnect(client);
}

// public
void Server::OnDisconnect(Client* client)
{
    const auto& clientInfo = client->GetInfoString();
    printf("\n%s Disconnected...", clientInfo.c_str());

    uint32_t id = client->GetID();
    auto iter = m_Clients.find(id);
    if (iter != m_Clients.end())
    {
        std::lock_guard guard(m_MutexClients);

        delete iter->second;
        iter->second = nullptr;
        m_Clients.erase(id);
    }
}

//private
void Server::WaitToAcceptNewConnection()
{
    try
    {
        printf("\nWaiting for New Connection...");
        m_Acceptor.async_accept([this](boost::system::error_code ec, tcp::socket socket)
            {
                if (ec)
                {
                    printf("Error on New Client Connection :%s", ec.message().c_str());
                    return;
                }

                Client* newClient = Client::Create(this, std::move(socket), m_NewClientID);
                printf("\nClient Connected : %s", newClient->GetInfoString().c_str());

                m_MutexClients.lock();
                m_Clients.insert(std::make_pair(m_NewClientID, newClient));
                m_MutexClients.unlock();

                ++m_NewClientID;

                newClient->ScheduleRead();

                WaitToAcceptNewConnection();
            });
    }
    catch (std::exception& e) 
    {
        printf("\n%s", e.what());
    }
}

// public
void Server::Write(Client* client, const std::vector<uint8_t>& buffer)
{
    Write(client, buffer, buffer.size());
}

// public
void Server::Write(Client* client, const std::vector<uint8_t>& buffer, std::size_t numBytesToWrite)
{
    client->ScheduleWrite(buffer, numBytesToWrite);
}


END_NAMESPACE_TCP
