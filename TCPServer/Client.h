#pragma once

#include "Common.h"
#include <boost/asio.hpp>

BEGIN_NAMESPACE_TCP

using boost::asio::ip::tcp;

class Server;

class Client
{
public:
    Client(const Client& rhs) = delete;
    Client(Client&&) = delete;

    ~Client();

    bool IsConnected() const;
    void ScheduleRead();

    void Write(const std::vector<uint8_t>& buffer, std::size_t bytesToWrite);
    void ScheduleWrite(const std::vector<uint8_t>& buffer, std::size_t bytesToWrite);

    std::string GetInfoString() const;

    const std::vector<uint8_t>& GetReadBuffer() const { return m_ReadBuffer; }
    std::size_t     GetBytesRead() const { return m_BytesRead; }
    uint32_t        GetID() const { return m_ID; }

    static Client* Create(Server* server, tcp::socket socket, uint32_t id);

private:
    Client(Server* server, tcp::socket socket, uint32_t id);

private:

    std::size_t                                 m_BytesRead;
    std::vector<uint8_t>                        m_ReadBuffer;
    Server*                                     m_Server;
    tcp::socket                                 m_Socket;
    const uint32_t                              m_ID;
};

END_NAMESPACE_TCP

