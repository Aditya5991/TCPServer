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

class Server
{
public:

    Server(int port, uint32_t maxClientsAllowed = -1);
    virtual ~Server();

    bool Start();
    bool Stop();

    virtual bool OnClientConnected(Client* client);
    virtual void OnDataReceived(Client* client);
    virtual void OnDataReceivedError(Client* client, const boost::system::error_code& ec);
    
    bool OnClientConnected(tcp::socket socket);
    void OnDisconnect(Client* client);

    void MessageAllClients(const std::vector<uint8_t>& message, Client* clientToIgnore = nullptr);
    void MessageAllClients(const std::vector<uint8_t>& message, std::size_t bytesToWrite, Client* clientToIgnore = nullptr);

    void Write(Client* client, const std::string& buffer);
    void Write(Client* client, const std::vector<uint8_t>& buffer);
    void Write(Client* client, const std::vector<uint8_t>& buffer, std::size_t numBytesToWrite);

    void AsyncWrite(Client* client, const std::string& buffer);
    void AsyncWrite(Client* client, const std::vector<uint8_t>& buffer);
    void AsyncWrite(Client* client, const std::vector<uint8_t>& buffer, std::size_t numBytesToWrite);

    void Wait();

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
