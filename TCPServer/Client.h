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

    /**
    * Checks if the socket is still open or not,
    * 
    * @return
    *       True, if the socket is open, else false.
    */
    bool IsConnected() const;

    /**
    * Add an asynchromnous task to read from this clien's socket.
    */
    void ScheduleRead();

    /**
    * Synchrounous call to write data to the socket.
    * 
    * @param [in] buffer
    *       Byte data to be written.
    * 
    * @param [in] bytesToWrite
    *       Number of bytes of data to be written from 'buffer'.
    */
    void Write(const std::vector<uint8_t>& buffer, std::size_t bytesToWrite);

    /**
    * Asynchrounous call to write data to the socket.
    * 
    * @param [in] buffer
    *       Byte data to be written.
    *
    * @param [in] bytesToWrite
    *       Number of bytes of data to be written from 'buffer'.
    */
    void ScheduleWrite(const std::vector<uint8_t>& buffer, std::size_t bytesToWrite);

    /**
    * Helper function details basic stats about the client.
    */
    std::string GetInfoString() const;

    /**
    * Returns the latest byte data that is read from the socket.
    */
    const std::vector<uint8_t>& GetReadBuffer() const { return m_ReadBuffer; }

    /**
    * Returns the latest number of bytes that are read into the 'm_ReadBuffer'.
    */
    std::size_t     GetBytesRead() const { return m_BytesRead; }

    /**
    * Returns the ID of this client that was assigned by the server.
    */
    uint32_t        GetID() const { return m_ID; }

    /**
    * Factory function to create an object of the class.
    * 
    * @params [in] server
    *       Pointer to the server handler class.
    * 
    * @params [in] socket
    *       Socket to which this client is connected to.
    * 
    * @params [in] id
    *       ID that is assigned by the server to this client.
    * 
    */
    static Client* Create(Server* server, tcp::socket socket, uint32_t id);

private:
    Client(Server* server, tcp::socket socket, uint32_t id);

private:

    /* Latest number of bytes that are read into the 'm_ReadBuffer'. */
    std::size_t                                 m_BytesRead;

    /* Latest bytes that are read into from the socket. */
    std::vector<uint8_t>                        m_ReadBuffer;

    /* Pointer to the server handler class. */
    Server*                                     m_Server;

    /* boost::asio::ip::tcp::socket object that is handled by this class. */
    tcp::socket                                 m_Socket;

    /* ID that is assigned to this client by the server. */
    const uint32_t                              m_ID;
};

END_NAMESPACE_TCP

