#pragma once

#include "Common.h"
#include <boost/asio.hpp>

BEGIN_NAMESPACE_TCP

using boost::asio::ip::tcp;

class Server;

class Client : std::enable_shared_from_this<Client>
{
public:
    Client(const Client& rhs) = delete;
    Client(Client&&) = delete;

    ~Client();

    bool IsConnected() const;
    void ScheduleRead();

    const std::vector<uint8_t>& GetReadBuffer() const;
    std::string GetInfoString() const;

    uint32_t GetID() const { return m_ID; }

    static Client* Create(Server* server, tcp::socket socket, uint32_t id);

private:
    Client(Server* server, tcp::socket socket, uint32_t id);

private:

    Server*                                     m_Server;
    std::vector<uint8_t>                        m_ReadBuffer;
    tcp::socket                                 m_Socket;
    const uint32_t                              m_ID;
};

END_NAMESPACE_TCP

