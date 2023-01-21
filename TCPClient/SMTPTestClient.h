#pragma once

#include "TCPClient.h"
#include <boost/bind/bind.hpp>

class SMTPClient : public net::tcp::Client
{
    using base = net::tcp::Client;

public:
    SMTPClient() {}

    bool OnResponse_StartTLS(const std::shared_ptr<const std::vector<uint8_t>>& buffer, std::size_t bytesRead)
    {
        std::string data(buffer->begin(), buffer->begin() + bytesRead);
        printf("\nFrom Server: %s", data.c_str());

        return true;
    }

    bool OnWrite_StartTLS(std::size_t bytesWritten)
    {
        auto callback = boost::bind(&SMTPClient::OnResponse_StartTLS, this, boost::placeholders::_1, boost::placeholders::_2);
        base::AsyncRead(callback);

        return true;
    }

    bool OnResponse_EHLO(const std::shared_ptr<const std::vector<uint8_t>>& buffer, std::size_t bytesRead)
    {
        std::string data(buffer->begin(), buffer->begin() + bytesRead);
        printf("\nFrom Server : %s", data.c_str());

        auto callback = boost::bind(&SMTPClient::OnWrite_StartTLS, this, boost::placeholders::_1);
        base::AsyncWrite("STARTTLS" CRLF, callback);

        return true;
    }

    bool OnWrite_EHLO(std::size_t bytesWritten)
    {
        // read
        base::AsyncRead(boost::bind(&SMTPClient::OnResponse_EHLO, this, boost::placeholders::_1, boost::placeholders::_2));

        return true;
    }

    virtual bool OnConnected() override
    {
        printf("\nConnected to the Server : %s, %d", GetServerHostname().c_str(), GetPort());

        std::string ehlo = "EHLO 192.168.0.109" CRLF;
        auto callback = boost::bind(&SMTPClient::OnWrite_EHLO, this, boost::placeholders::_1);
        base::AsyncWrite(ehlo, callback);

        return true;
    }

    virtual void OnConnectionError(const std::string& errorMessage)
    {
        printf("\nError Connecting to Server : %s", errorMessage.c_str());
    }

    virtual bool OnDataReceived(
        const std::shared_ptr<const std::vector<uint8_t>>& buffer,
        std::size_t bytesRead) override
    {
        static int stepCount = 1;

        const auto& b = *buffer;
        std::string data(b.begin(), b.begin() + bytesRead);
        printf("\nFrom Server : %s", data.c_str());

        if (stepCount == 1)
        {
            std::string ehlo = "EHLO 192.168.0.109" CRLF;
            base::AsyncWrite(ehlo);
        }
        else if (stepCount == 2)
        {
            base::AsyncWrite("STARTTLS" CRLF);
        }
        else if (stepCount == 3)
        {
            base::AsyncWrite("client hello" CRLF);
        }

        ++stepCount;

        return true;
    }


    virtual void OnDataReceivedError(const std::string& errorMessage) override
    {
        printf("\nError occured while reading data : %s", errorMessage.c_str());
    }


    virtual void OnDataWritten(std::size_t bytesWritten) override
    {
        printf("\nWritten %" SCNu64 " bytes to server." CRLF, bytesWritten);
    }

    virtual void OnDisconnection() override
    {
        printf("\nDisconnected from the server.");
    }

};