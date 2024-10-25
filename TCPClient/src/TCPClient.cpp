#include "TCPClient.h"

BEGIN_NAMESPACE_TCP

// public
Client::Client()
    : m_Socket(m_IOContext)
    , m_ServerHostname("")
    , m_Port(-1)
{
}

// public
bool Client::AsyncConnect(
    const std::string& serverHostname, 
    uint16_t port)
{
    m_ServerHostname = serverHostname;
    m_Port = port;

    try
    {
        boost::asio::ip::tcp::resolver resolver(GetIOContext());
        boost::asio::ip::tcp::resolver::query query(GetServerHostname(), std::to_string(GetPort()));
        boost::asio::ip::tcp::endpoint serverEndpoint = *resolver.resolve(query);

        /* Add a task for connecting to the server before starting the Context Thread. */
        GetSocket().async_connect(serverEndpoint,
            [this](const boost::system::error_code& ec)
            {
                if (ec)
                {
                    OnConnectionError(ec.message());
                    return;
                }

                /* will call the derived class's function. */
                if (!OnConnected())
                    return;
            });
    }
    catch (std::exception& e)
    {
        printf("\n%s", e.what());
        return false;
    }

    GetContextThread() = std::thread([this]() {GetIOContext().run(); });

    return true;
}

// public
bool Client::AsyncRead(OnDataReceivedCallback callback)
{
    std::shared_ptr<std::vector<uint8_t>> buffer = std::make_shared<std::vector<uint8_t>>(1024);
    GetSocket().async_read_some(boost::asio::buffer(*buffer),
        [this, buffer, callback](const boost::system::error_code& ec, std::size_t bytesRead)
        {
            if (ec == boost::asio::error::eof)
            {
                OnDisconnection();
                return;
            }

            if (ec)
            {
                OnDataReceivedError(ec.message());
                return;
            }

            if (callback)
                callback(buffer, bytesRead);
            else
                OnDataReceived(buffer, bytesRead);

            /* Add another async read task to the context. */
            AsyncRead();
        });

    return true;
}

// public
void Client::AsyncWrite(const std::string& message, OnDataWrittenCallback callback)
{
    std::vector<uint8_t> buffer(message.begin(), message.end());
    AsyncWrite(buffer, buffer.size(), callback);
}

// public
void Client::AsyncWrite(const std::vector<uint8_t>& buffer, std::size_t bytesToWrite, OnDataWrittenCallback callback)
{
    if (bytesToWrite == 0)
        bytesToWrite = buffer.size();

    GetSocket().async_write_some(boost::asio::buffer(buffer.data(), bytesToWrite),
        [this, callback](const boost::system::error_code& ec, std::size_t bytesWritten)
        {
            if (ec)
            {
                OnDataWritingError(ec.message());
                return;
            }

            if (callback)
                callback(bytesWritten);
            else
                OnDataWritten(bytesWritten);
        });
}

// public
void Client::Wait()
{
    if (GetContextThread().joinable())
        GetContextThread().join();
}


END_NAMESPACE_TCP

