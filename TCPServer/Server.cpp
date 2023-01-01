#include "Server.h"
#include "ClientHandler.h"
#include "IOBuffer.h"
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
void Server::OnDataReceivedError(ClientID clientID, const boost::system::error_code& ec)
{
    /* directly disconnect the connection of this client */
    OnClientDisconnected(clientID);
}

// public
void Server::OnClientDisconnected(ClientID clientID)
{
    ClientHandler* clientHandle = m_ClientHandlers[clientID];
    const auto& clientInfo = clientHandle->GetInfoString();

    uint32_t id = clientHandle->GetID();
    auto iter = m_ClientHandlers.find(id);
    if (iter == m_ClientHandlers.end())
        return;
     
    // remove the client from our clients map
    std::lock_guard guard(m_MutexClients);
    delete iter->second;
    iter->second = nullptr;
    m_ClientHandlers.erase(id);
}

// public virtual
bool Server::OnClientConnected(tcp::socket socket)
{
    // check if max client limit is reached
    if (m_ClientHandlers.size() >= m_MaxClientsAllowed)
    {
        printf("\nMax Clients Allowed Limit Reached!");
        std::string message = "The Server has reached the Maximum number of allowed Clients Limit!\nYou will be disconnected!";
        Write(socket, message);
        return false;
    }

    // first level of checking is done, so we can create a client handler object.
    ClientHandler* newClientHandle = ClientHandler::Create(this, std::move(socket), m_NewClientID);

    // add the new client to the clients map.
    m_MutexClients.lock();
    m_ClientHandlers.insert(std::make_pair(m_NewClientID, newClientHandle));
    m_MutexClients.unlock();

    // will call the derived classes func, for further checking.
    if (!OnClientConnected(newClientHandle->GetID()))
    {
        // remove the new client from the clients map.
        m_MutexClients.lock();
        m_ClientHandlers.erase(m_NewClientID);
        m_MutexClients.unlock();

        printf("\n Connection refused to : %s", newClientHandle->GetInfoString().c_str());
        delete newClientHandle;
        return false;
    }

    ++m_NewClientID;

    // start an async task for reading data from the newClient
    newClientHandle->ScheduleRead();

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

                // schedule the next task to accept new connection.
                WaitToAcceptNewConnection();
            });
    }
    catch (std::exception& e) 
    {
        printf("\n%s", e.what());
    }
}

// public
void Server::Write(tcp::socket& socket, const std::string& buffer)
{
    if (!socket.is_open())
        return;

    try
    {
        socket.write_some(boost::asio::buffer(buffer.data(), buffer.size()));
    }
    catch (std::exception& e)
    {
        printf("\n%s", e.what());
    }
}

// public
void Server::Write(ClientID ID, const std::string& str)
{
    std::vector<uint8_t> buffer(str.begin(), str.end());
    Write(ID, buffer);
}

// public
void Server::Write(ClientID ID, const std::vector<uint8_t>& buffer)
{
    Write(ID, buffer, buffer.size());
}

// public
void Server::Write(ClientID ID, const std::vector<uint8_t>& buffer, std::size_t numBytesToWrite)
{
    ClientHandler* clientHandle = m_ClientHandlers[ID];
    clientHandle->Write(buffer, numBytesToWrite);
}

// public
void Server::AsyncWrite(ClientID ID, const std::string& str)
{
    std::vector<uint8_t> buffer(str.begin(), str.end());
    AsyncWrite(ID, buffer);
}

// public
void Server::AsyncWrite(ClientID ID, const std::vector<uint8_t>& buffer)
{
    AsyncWrite(ID, buffer, buffer.size());
}

// public
void Server::AsyncWrite(ClientID ID, const std::vector<uint8_t>& buffer, std::size_t numBytesToWrite)
{
    ClientHandler* client = m_ClientHandlers[ID];
    client->ScheduleWrite(buffer, numBytesToWrite);
}

// public
void Server::MessageClient(ClientID ID, const std::string& message)
{
    std::vector<uint8_t> buffer(message.begin(), message.end());
    MessageClient(ID, buffer, buffer.size());
}

// public
void Server::MessageClient(ClientID ID, const net::IOBuffer& buffer)
{
    if (buffer.HasData())
        MessageClient(ID, buffer.GetData(), buffer.Size());
}

// public
void Server::MessageClient(ClientID ID, const std::vector<uint8_t>& buffer, std::size_t bytesToWrite)
{
    ClientHandler* client = m_ClientHandlers[ID];
    client->ScheduleWrite(buffer, bytesToWrite);
}

// public
void Server::MessageAllClients(const net::IOBuffer& buffer, ClientID clientToIgnoreID)
{
    for (auto iter : m_ClientHandlers)
    {
        std::size_t ID = iter.first;
        if (IsValidClientID(clientToIgnoreID) && ID == clientToIgnoreID)
            continue;

        MessageClient(ID, buffer);
    }
}

// public
void Server::MessageAllClients(const std::vector<uint8_t>& buffer, ClientID clientToIgnoreID)
{
    MessageAllClients(buffer, buffer.size(), clientToIgnoreID);
}

// public
void Server::MessageAllClients(const std::vector<uint8_t>& message, std::size_t bytesToWrite, ClientID clientToIgnoreID)
{
    for (auto iter : m_ClientHandlers)
    {
        std::size_t ID = iter.first;
        if (IsValidClientID(clientToIgnoreID) && ID == clientToIgnoreID)
            continue;

        MessageClient(ID, message, bytesToWrite);
    }
}

// public
void Server::MessageAllClients(const std::string& message, ClientID clientToIgnoreID)
{
    std::vector<uint8_t> buffer(message.begin(), message.end());
    MessageAllClients(buffer, buffer.size(), clientToIgnoreID);
}

// public
void Server::Wait()
{
    /* wait for the io_context to run out of jobs to perform */
    if (m_ContextThread.joinable())
        m_ContextThread.join();
}

END_NAMESPACE_TCP
