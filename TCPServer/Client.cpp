#include "Client.h"

using boost::asio::ip::tcp;

BEGIN_NAMESPACE_TCP

// public
Client::Client(
    boost::asio::io_context& context, 
    tcp::socket socket)
    : m_ReadBuffer(1 * 1024)
    , m_IOContext(context)
    , m_Socket(std::move(socket))
{
}

// public
bool Client::IsConnected() const
{
    return m_Socket.is_open();
}

// public
void Client::ScheduleRead()
{
    m_Socket.async_read_some(boost::asio::buffer(m_ReadBuffer.data(), m_ReadBuffer.size()),
        [this](const boost::system::error_code& ec, std::size_t bytesRead) 
        {
            if (ec == boost::asio::error::eof)
            {
                printf("\n%s Disconnected...", GetInfoString().c_str());
                return;
            }

            if (ec)
            {
                printf("Error reading from Client : %s", ec.message().c_str());
                return;
            }

            std::string data(m_ReadBuffer.begin(), m_ReadBuffer.begin() + bytesRead);
            printf("\nFrom %s : %s", GetInfoString().c_str(), data.c_str());

            ScheduleRead();
        });
}

// public
std::string Client::GetInfoString() const
{
    std::string ip = m_Socket.remote_endpoint().address().to_string();
    int port = m_Socket.remote_endpoint().port();

    return std::format("{}({})", ip, port);
}


END_NAMESPACE_TCP
