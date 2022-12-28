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
Server::Server(int port, uint32_t maxClientsAllowed)
    : m_MaxClientsAllowed(maxClientsAllowed)
    , m_Port(port)
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

// public virtual
bool Server::OnClientConnected(tcp::socket socket)
{
    Client* newClient = Client::Create(this, std::move(socket), m_NewClientID);
    
    if (m_Clients.size() >= m_MaxClientsAllowed)
    {
        printf("\nMax Clients Allowed Limit Reached!");
        std::string message = "The Server has reached the Maximum number of allowed Clients Limit!\nYou will be disconnected!";
        Write(newClient, message);
        delete newClient;
        return false;
    }

    printf("\nClient Connected : %s", newClient->GetInfoString().c_str());

    m_MutexClients.lock();
    m_Clients.insert(std::make_pair(m_NewClientID, newClient));
    m_MutexClients.unlock();

    ++m_NewClientID;

    newClient->ScheduleRead();

    return true;
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

                OnClientConnected(std::move(socket));

                WaitToAcceptNewConnection();
            });
    }
    catch (std::exception& e) 
    {
        printf("\n%s", e.what());
    }
}


// public
void Server::Write(Client* client, const std::string& str)
{
    std::vector<uint8_t> buffer(str.begin(), str.end());
    Write(client, buffer);
}

// public
void Server::Write(Client* client, const std::vector<uint8_t>& buffer)
{
    Write(client, buffer, buffer.size());
}

// public
void Server::Write(Client* client, const std::vector<uint8_t>& buffer, std::size_t numBytesToWrite)
{
    client->Write(buffer, numBytesToWrite);
}

// public
void Server::AsyncWrite(Client* client, const std::string& str)
{
    std::vector<uint8_t> buffer(str.begin(), str.end());
    AsyncWrite(client, buffer);
}

// public
void Server::AsyncWrite(Client* client, const std::vector<uint8_t>& buffer)
{
    AsyncWrite(client, buffer, buffer.size());
}

// public
void Server::AsyncWrite(Client* client, const std::vector<uint8_t>& buffer, std::size_t numBytesToWrite)
{
    client->ScheduleWrite(buffer, numBytesToWrite);
}


END_NAMESPACE_TCP
