#pragma once

#include "Server.h"
#include <boost/asio.hpp>

BEGIN_NAMESPACE_TCP

class ClientHandler
{
public:
    ClientHandler(
        boost::asio::ip::tcp::socket socket, 
        uint32_t id,
        OnDataReceivedCallback cb_OnDataReceived,
        OnDataReceivedErrorCallback cb_OnDataReceivedError,
        OnClientDisconnectedCallback cb_OnClientDisconnected
    );

    ClientHandler(const ClientHandler& rhs) = delete;
    ClientHandler(ClientHandler&&) = delete;

    ~ClientHandler();

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
    * @param [in] server
    *       Pointer to the server handler class.
    * 
    * @param [in] socket
    *       Socket to which this client is connected to.
    * 
    * @param [in] id
    *       ID that is assigned by the server to this client.
    * 
    * @param [in] cb_OnDataReceived
    *       Callback function, called when server receives any data from the client.
    * 
    * @param [in] cb_OnDataReceivedError
    *       Callback function, called when server receives any errorneous data from the client.
    * 
    * @param [in] cb_OnClientDisconnected
    *       Callback function, called when a client disconnects from the server.
    *
    */
    static ClientHandlerSPtr Create(
        boost::asio::ip::tcp::socket socket, 
        uint32_t id,
        OnDataReceivedCallback cb_OnDataReceived,
        OnDataReceivedErrorCallback cb_OnDataReceivedError,
        OnClientDisconnectedCallback cb_OnClientDisconnected
    );

private:

    /* Latest number of bytes that are read into the 'm_ReadBuffer'. */
    std::size_t                                 m_BytesRead;

    /* Latest bytes that are read into from the socket. */
    std::vector<uint8_t>                        m_ReadBuffer;

    /* boost::asio::ip::tcp::socket object that is handled by this class. */
    boost::asio::ip::tcp::socket                m_Socket;

    /* ID that is assigned to this client by the server. */
    const uint32_t                              m_ID;

    /* Callbacks */
    /* This callback will be called after the server receives data from the client. */
    OnDataReceivedCallback m_OnDataReceivedCallback;

    /* This callback will be called if we receive any errorneous data from the client. */
    OnDataReceivedErrorCallback m_OnDataReceivedErrorCallback;

    /* This callback will be called when the client disconnects. */
    OnClientDisconnectedCallback m_OnClientDisconnectedCallback;

};


END_NAMESPACE_TCP

