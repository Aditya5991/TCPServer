#pragma once

#include "Common.h"
#include <boost/asio.hpp>
#include <thread>
#include <mutex>
#include <array>
#include <inttypes.h>


BEGIN_NAMESPACE_TCP

class Client;

class Server
{
public:

    Server(int port);
    ~Server();

    bool Start();
    bool Stop();

    int GetPort() const { return m_Port; }

private:
    void WaitToAcceptNewConnection();

    boost::asio::io_context& IOContext() { return m_IOContext; }

private:
    int                                             m_Port;

    boost::asio::io_context                         m_IOContext;
    std::thread                                     m_ContextThread;

    boost::asio::ip::tcp::acceptor                  m_Acceptor;
    std::vector<std::shared_ptr<TCP::Client>>       m_Clients;
};

END_NAMESPACE_TCP
