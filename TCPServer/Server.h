#pragma once

#include "Common.h"
#include <boost/asio.hpp>
#include <thread>
#include <mutex>
#include <queue>
#include <inttypes.h>


BEGIN_NAMESPACE_TCP

using boost::asio::ip::tcp;

class Client;

class Server : std::enable_shared_from_this<Server>
{
public:

    Server(int port, uint32_t maxClientsAllowed = -1);
    ~Server();

    bool Start();
    bool Stop();

    virtual bool OnClientConnected(tcp::socket socket);
    virtual void OnDataReceived(Client* client, std::size_t bytesRead);
    virtual void OnDataReceivedError(Client* client, const boost::system::error_code& ec);
    virtual void OnDisconnect(Client* client);

    void Write(Client* client, const std::string& buffer);
    void Write(Client* client, const std::vector<uint8_t>& buffer);
    void Write(Client* client, const std::vector<uint8_t>& buffer, std::size_t numBytesToWrite);

    void AsyncWrite(Client* client, const std::string& buffer);
    void AsyncWrite(Client* client, const std::vector<uint8_t>& buffer);
    void AsyncWrite(Client* client, const std::vector<uint8_t>& buffer, std::size_t numBytesToWrite);

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
    uint32_t                                        m_MaxClientsAllowed;
};

END_NAMESPACE_TCP
