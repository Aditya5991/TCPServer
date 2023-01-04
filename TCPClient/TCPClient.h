#pragma once

#include "TCPCommon/Common.h"
#include <boost/asio.hpp>


BEGIN_NAMESPACE_TCP


/**
* This class gives a basic implementation of a TCP Client.
* It supports Synchronous and Asynchronous Read/Write operations.
*
* Override this class to give your own implementations to the relevant member functions.
*/
class Client
{
public:

    /**
    * Default Constructor
    */
    Client();

    /**
    * Delete the copy constructor.
    */
    Client(const Client&) = delete;

    /**
    * This function starts an asynchronous task to connect to the server
    * and will call the OnConnected function.
    * 
    * @param [in] serverHostname
    *       Pame of the server to connect to.
    *
    * @param [in] port
    *       Port on which the server is listening for new connections.
    * 
    * @return
    *       True, always for now.
    */
    bool AsyncConnect(const std::string& serverHostname, uint16_t port);

    /**
    * This function should to be overridden to handle what to do once a connection is established.
    * 
    * @return
    *       False
    */
    virtual bool OnConnected() { return false; }

    /**
    * This function will be called if there was an error while connecting to the server from AsyncConnect() func.
    * 
    * @param [in] errorMessage
    *       Error Message returned by boost::asio.
    */
    virtual void OnConnectionError(const std::string& errorMessage) { }

    /**
    * This function starts an asynchronous task to read data from the server.
    * 
    * @return
    *       True, always for now.
    */
    bool AsyncRead();

    /**
    * This function start an async task to write data to the server.
    * 
    * @param [in] message
    *       Message to be written to the server in the form of std::string.
    */
    void AsyncWrite(const std::string& message);

    /**
    * This function start an async task to write data to the server.
    *
    * @param [in] buffer
    *       Data to be written to the server in the form of byte stream.
    * 
    * @param [in] bytesToWrite
    *       Number of bytes from the 'buffer' to be written to the server.
    *       If 0, then the whole buffer will be written to the server.
    */
    void AsyncWrite(const std::vector<uint8_t>& buffer, std::size_t bytesToWrite = 0);

    /**
    * This function will be called if boost::asio detects any error while writing data to the server.
    * 
    * @param [in] errorMessage
    *       Error Message provided by boost::asio.
    */
    virtual void OnDataWritingError(const std::string& errorMessage) {}

    /**
    * This function will be called when data is successfully written to the server.
    * 
    * @param [in] bytesWritten
    *       The number of bytes written to the server.
    */
    virtual void OnDataWritten(std::size_t bytesWritten) {}

    /**
    * This is function is called when any data is received from the server.
    * 
    * @param [in] buffer
    *       A shared pointer to the byte data that has been read from the server.
    * 
    * @param [in] bytesRead
    *       Number of bytes read from the server.
    * 
    * @return
    *       True, always for now.
    */
    virtual bool OnDataReceived(const std::shared_ptr<const std::vector<uint8_t>>& buffer, std::size_t bytesRead) 
    { (void)buffer; (void)bytesRead; return false; }

    /**
    * This function is called when boost::asio says that and error has occurred while reading data from the server.
    * 
    * @param [in] errorMessage
    *       Error Message returned by boost::asio.
    * 
    */
    virtual void OnDataReceivedError(const std::string& errorMessage) { (void)errorMessage; }

    /**
    * This function is called when the server disconnects from the client.
    */
    virtual void OnDisconnection() {}

    /**
    * Wait for the Context thread to finish its tasks.
    */
    void Wait();

    /**
    * Getter for ServerHostName.
    */
    const std::string& GetServerHostname() const { return m_ServerHostname; }

    /**
    * Getter for Server Port.
    */
    uint16_t GetPort() const { return m_Port; }

private:
    
    std::thread& GetContextThread() { return m_ContextThread; }
    boost::asio::io_context& GetIOContext() { return m_IOContext; }
    boost::asio::ip::tcp::socket& GetSocket() { return m_Socket; }

private:

    /* Context on which all the network transactions will take place from boost. */
    boost::asio::io_context             m_IOContext;

    /* Socket on which the client is connected to the server. */
    boost::asio::ip::tcp::socket        m_Socket;

    /* Thread on which the asynchronous task will be performed by the io_context */
    std::thread                         m_ContextThread;

    /* Hostname of the server that the client will connect to. */
    std::string                         m_ServerHostname;

    /* Port on which the server is listening for new connections. */
    uint16_t                            m_Port;

};

END_NAMESPACE_TCP


