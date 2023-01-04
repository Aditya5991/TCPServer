#include "TCPClient.h"

BEGIN_NAMESPACE_TCP

// public
Client::Client()
    : m_Socket(m_IOContext)
{
}


// public
bool Client::AsyncRead()
{
    std::shared_ptr<std::vector<uint8_t>> buffer = std::make_shared<std::vector<uint8_t>>(1024);
    GetSocket().async_read_some(boost::asio::buffer(*buffer.get()),
        [this, buffer](const boost::system::error_code& ec, std::size_t bytesRead)
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

            if (!OnDataReceived(buffer, bytesRead))
                return;

            /* Add another async read task to the context. */
            AsyncRead();
        });

    return true;
}

// public
bool Client::AsyncConnect(const std::string& serverHostname, uint16_t port)
{
    m_ServerHostname = serverHostname;
    m_Port = port;
    m_ServerEndpoint = { boost::asio::ip::address::from_string(GetServerHostname()), GetPort() };

    /* Add a task for connecting to the server before starting the Context Thread. */
    GetSocket().async_connect(m_ServerEndpoint,
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

            /* Add a async read task to the context. */
            AsyncRead();
        });

    GetContextThread() = std::thread([this]() {GetIOContext().run(); });

    return true;
}

void Client::Wait()
{
    if (GetContextThread().joinable())
        GetContextThread().join();
}

END_NAMESPACE_TCP

