#include "ClientHandler.h"
#include "Server.h"

using boost::asio::ip::tcp;

BEGIN_NAMESPACE_TCP

// private
ClientHandler::ClientHandler(
    Server* server,
    tcp::socket socket,
    uint32_t id)
    : m_BytesRead(0)
    , m_ReadBuffer(1 * 1024)
    , m_Server(server)
    , m_Socket(std::move(socket))
    , m_ID(id)
{
}

// public
ClientHandler::~ClientHandler()
{
    m_Socket.close();
}

// public static
ClientHandler* ClientHandler::Create(Server* server, tcp::socket socket, uint32_t id)
{
    return new ClientHandler(server, std::move(socket), id);
}

// public
bool ClientHandler::IsConnected() const
{
    return m_Socket.is_open();
}

// public
void ClientHandler::ScheduleRead()
{
    if (!IsConnected())
        return;

    m_Socket.async_read_some(boost::asio::buffer(m_ReadBuffer.data(), m_ReadBuffer.size()),
        [this](const boost::system::error_code& ec, std::size_t bytesRead)
        {
            // check if the client disconnected from the server.
            if (ec == boost::asio::error::eof)
            {
                m_Server->OnClientDisconnected(GetID());
                return;
            }

            // check if any errorneous data(according to boost::asio) is received from the client.
            if (ec)
            {
                m_Server->OnDataReceivedError(GetID(), ec);
                return;
            }

            m_BytesRead = bytesRead;
            m_Server->OnDataReceived(GetID());

            /* Assign the context with another read task. */
            ScheduleRead();
        });
}

// public
void ClientHandler::Write(const std::vector<uint8_t>& buffer, std::size_t bytesToWrite)
{
    if (!IsConnected())
        return;

    try 
    {
        m_Socket.write_some(boost::asio::buffer(buffer.data(), bytesToWrite));
    }
    catch (std::exception& e)
    {
        printf("\n%s", e.what());
    }
}

// public
void ClientHandler::ScheduleWrite(const std::vector<uint8_t>& buffer, std::size_t bytesToWrite)
{
    if (!IsConnected())
        return;

    m_Socket.async_write_some(boost::asio::buffer(buffer.data(), bytesToWrite),
        [this](const boost::system::error_code& ec, std::size_t bytesWritten)
        {
            if (ec)
            {
                printf("\nError Writing to %s.", GetInfoString().c_str());
                return;
            }
        });
}

// public
std::string ClientHandler::GetInfoString() const
{
    std::string ip = m_Socket.remote_endpoint().address().to_string();
    int port = m_Socket.remote_endpoint().port();

    return std::format("[{}] {}({})", GetID(), ip, port);
}


END_NAMESPACE_TCP
