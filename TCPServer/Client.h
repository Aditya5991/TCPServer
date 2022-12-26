#pragma once

#include "Common.h"
#include <boost/asio.hpp>

BEGIN_NAMESPACE_TCP

using boost::asio::ip::tcp;

class Client
{
public:
    Client(boost::asio::io_context& context, tcp::socket socket);

    bool IsConnected() const;
    void ScheduleRead();

    std::string GetInfoString() const;

private:

    std::vector<uint8_t>            m_ReadBuffer;
    boost::asio::io_context&        m_IOContext;
    tcp::socket                     m_Socket;
};

END_NAMESPACE_TCP

