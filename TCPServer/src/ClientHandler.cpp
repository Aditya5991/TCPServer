#include "ClientHandler.h"

using boost::asio::ip::tcp;

BEGIN_NAMESPACE_TCP

// private
ClientHandler::ClientHandler(
    boost::asio::ip::tcp::socket socket,
    uint32_t id,
    OnDataReceivedCallback cb_OnDataReceived,
    OnDataReceivedErrorCallback cb_OnDataReceivedError,
    OnClientDisconnectedCallback cb_OnClientDisconnected
    )
    : m_BytesRead(0)
    , m_ReadBuffer(1 * 1024)
    , m_Socket(std::move(socket))
    , m_ID(id)
    , m_OnDataReceivedCallback(cb_OnDataReceived)
    , m_OnDataReceivedErrorCallback(cb_OnDataReceivedError)
    , m_OnClientDisconnectedCallback(cb_OnClientDisconnected)

{
}

// public
ClientHandler::~ClientHandler()
{
    m_Socket.close();
}

// public static
ClientHandlerSPtr ClientHandler::Create(
    boost::asio::ip::tcp::socket socket,
    uint32_t id,
    OnDataReceivedCallback cb_OnDataReceived,
    OnDataReceivedErrorCallback cb_OnDataReceivedError,
    OnClientDisconnectedCallback cb_OnClientDisconnected
)
{
    return std::make_shared<ClientHandler>(std::move(socket), id, cb_OnDataReceived, cb_OnDataReceivedError, cb_OnClientDisconnected);
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
                m_OnClientDisconnectedCallback(GetID());
                //m_Server->OnClientDisconnected(GetID());
                return;
            }

            // check if any errorneous data(according to boost::asio) is received from the client.
            if (ec)
            {
                m_OnDataReceivedErrorCallback(GetID(), ec);
                //m_Server->OnDataReceivedError(GetID(), ec);
                return;
            }

            m_BytesRead = bytesRead;
            m_OnDataReceivedCallback(GetID());
            //m_Server->OnDataReceived(GetID());

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
