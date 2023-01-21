#pragma once

#include "TCPClient.h"

class SimpleTCPClient : public net::tcp::Client
{
    using base = net::tcp::Client;
public:

    virtual bool OnConnected() override
    {
        // start an aync read task;
        AsyncRead();
        return true;
    }

    virtual bool OnDataReceived(
        const std::shared_ptr<const std::vector<uint8_t>>& buffer,
        std::size_t bytesRead) override
    {
        std::string data(buffer->begin(), buffer->end());
        printf("\nS : %s", data.c_str());
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


