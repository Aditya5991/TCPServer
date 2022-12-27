#pragma once

#include "Common.h"
#include <boost/asio.hpp>
#include <thread>
#include <mutex>
#include <queue>
#include <inttypes.h>


BEGIN_NAMESPACE_TCP

class Client;

class Server : std::enable_shared_from_this<Server>
{
public:

    Server(int port);
    ~Server();

    bool Start();
    bool Stop();

    void OnDataReceived(Client* client, std::size_t bytesRead);
    void OnDisconnect(Client* client);

    std::size_t GetNumClients() const { return m_Clients.size(); }
    int GetPort() const { return m_Port; }

private:
    void WaitToAcceptNewConnection();

    boost::asio::io_context& IOContext() { return m_IOContext; }

private:
    int                                             m_Port;

    boost::asio::io_context        m_IOContext;
    std::thread                                     m_ContextThread;
    boost::asio::ip::tcp::acceptor                  m_Acceptor;

    std::mutex                                      m_MutexClients;
    std::unordered_map<int, Client*>                m_Clients;
    uint32_t                                        m_NewClientID;
};

END_NAMESPACE_TCP
