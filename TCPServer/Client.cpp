#include "Client.h"
#include "Server.h"

using boost::asio::ip::tcp;

BEGIN_NAMESPACE_TCP

// private
Client::Client(
    Server* server,
    tcp::socket socket,
    uint32_t id)
    : m_ReadBuffer(1 * 1024)
    , m_Server(server)
    , m_Socket(std::move(socket))
    , m_ID(id)
{
}

// public
Client::~Client()
{
    m_Socket.close();
}

// public static
Client* Client::Create(Server* server, tcp::socket socket, uint32_t id)
{
    return new Client(server, std::move(socket), id);
}

// public
bool Client::IsConnected() const
{
    return m_Socket.is_open();
}

// public
void Client::ScheduleRead()
{
    if (!IsConnected())
        return;

    m_Socket.async_read_some(boost::asio::buffer(m_ReadBuffer.data(), m_ReadBuffer.size()),
        [this](const boost::system::error_code& ec, std::size_t bytesRead)
        {
            std::size_t availableBytes = m_Socket.available();

            if (ec == boost::asio::error::eof)
            {
                m_Server->OnDisconnect(this);
                return;
            }

            if (ec)
            {
                m_Server->OnDataReceivedError(this, ec);
                return;
            }

            m_Server->OnDataReceived(this, bytesRead);

            ScheduleRead();
        });
}

// public
void Client::ScheduleWrite(const std::vector<uint8_t>& buffer, std::size_t bytesToWrite)
{
    if (!IsConnected())
        return;

    m_Socket.async_write_some(boost::asio::buffer(buffer.data(), bytesToWrite),
        [this](const boost::system::error_code& ec, std::size_t bytesWritten)
        {
            printf("\nSent to %s", GetInfoString().c_str());
        });
}

// public
std::string Client::GetInfoString() const
{
    std::string ip = m_Socket.remote_endpoint().address().to_string();
    int port = m_Socket.remote_endpoint().port();

    return std::format("[{}] {}({})", GetID(), ip, port);
}

// public
const std::vector<uint8_t>& Client::GetReadBuffer() const
{
    return m_ReadBuffer;
}


END_NAMESPACE_TCP
