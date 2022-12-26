#include "Server.h"
#include "Client.h"
#include <iostream>
#include <vector>
#include <array>
#include <boost/asio.hpp>
#include <boost/bind/bind.hpp>

using boost::asio::ip::tcp;

BEGIN_NAMESPACE_TCP

// public
Server::Server(int port)
    : m_Port(port)
    , m_IOContext()
    , m_Acceptor(m_IOContext, tcp::endpoint(tcp::v4(), port))
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
    // order is important, as the context needs something to do, else it will end the context thread.
    WaitToAcceptNewConnection();
    m_ContextThread = std::thread([this](){ m_IOContext.run(); });

    return false;
}

// public
bool Server::Stop()
{
    if(m_ContextThread.joinable())
        m_ContextThread.join();

    m_IOContext.stop();

    return false;
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

                auto newClient = std::make_shared<Client>(m_IOContext, std::move(socket));
                printf("\nClient Connected : %s", newClient->GetInfoString().c_str());

                newClient->ScheduleRead();
                m_Clients.push_back(newClient);
                WaitToAcceptNewConnection();
            });
    }
    catch (std::exception& e) 
    {
        printf("\n%s", e.what());
    }
}

END_NAMESPACE_TCP
