#pragma once

#include "TCPCommon/Common.h"
#include <boost/asio.hpp>

namespace net { class IOBuffer; }

BEGIN_NAMESPACE_TCP

using OnDataReceivedCallback = std::function<void(ClientID)>;
using OnClientDisconnectedCallback = std::function<void(ClientID)>;
using OnDataReceivedErrorCallback = std::function<void(ClientID, const boost::system::error_code&)>;

class Server : public std::enable_shared_from_this<Server>
{
protected:
    Server(int port, uint32_t maxClientsAllowed = -1);

public:
    virtual ~Server();

    /**
    * Starts the async operation to accept client connections and then starts the Context Thread.
    * This order is important as without any task, the context thread would return.
    */
    bool Start();

    /**
    * Stops the server.
    */
    bool Stop();

    /**
    * This function is a callback function which is called when a new client is connected to the server.
    * 
    * @param[in] client
    *       const Pointer to client handler class that has just connected to this server.
    * 
    * @return
    *       return value indicates whether the server should accept the connection or not.
    *       e.g. Validating a client.
    */
    virtual bool OnClientConnected(ClientID ID) = 0;

    /**
    * This function is a callback which is called when data is received for any client.
    * 
    * @params [in] clientID
    *       ID of the client that has just connected to this server.
    * 
    */
    virtual void OnDataReceived(ClientID ID) = 0;

    /**
    * This function is called when any errorneous data is received from any client.
    * For now, this function disconnects the client directly.
    * 
    * @params [in] client
    *       ID of the client from which the errorneous data is received.
    * 
    * @params [in] ec
    *       error code received from boost::asio.
    */
    void OnDataReceivedError(ClientID ID, const boost::system::error_code& ec);
    
    /**
    * This function is called when any new client connects to the server and adds it to the client map.
    * Performs basic checks on whether or allow this new connection.
    * 
    * @params [in] socket
    *       boost::asio::ip::tcp::socket object, that is a handle to the newly connected client.
    * 
    * @return
    *       return value indicates whether the server should accept the connection or not.
    */
    bool OnClientConnected(boost::asio::ip::tcp::socket socket);

    /**
    * This function is called when a client disconnects from the server.
    * Removes the client from the clients map.
    * 
    * @params [in] client
    *       ID of the client that disconnected from the server.
    */
    virtual void OnClientDisconnected(ClientID ID);

    /**
    * This function can be used to send byte data to a specific Client.
    *
    * @param [in] ID
    *       ID of the client to send the data to.
    *
    * @param [in] buffer
    *       net::IOBuffer object that contains the data to be sent.
    */
    void MessageClient(ClientID ID, const IOBuffer& buffer);

    /**
    * This function can be used to send string data to a specific Client.
    *
    * @param [in] ID
    *       ID of the client to send the data to.
    *
    * @param [in] message
    *       String data that is to be sent to the Client.
    */
    void MessageClient(ClientID ID, const std::string& message);


    /**
    * This function can be used to send a buffer to a specific Client.
    *
    * @param [in] ID
    *       ID of the client to send the data to.
    *
    * @param [in] buffer
    *       Byte data that is to be sent to the Client.
    * 
    * @params [in] numBytesToWrite
    *       Number of bytes of write from the 'buffer' vector.
    */
    void MessageClient(ClientID ID, const std::vector<uint8_t>& buffer, std::size_t numBytesToWrite);

    /**
    * This function can be used to send a buffer in the form of IOBuffer to all the clients that are connected to this server.
    *
    * @params [in] buffer
    *       Bytes of data that needs to be sent.
    *
    * @param [in] clientToIgnoreID
    *       Optional param, ID of the client that we want to ignore sending the buffer to.
    *
    */
    void MessageAllClients(const net::IOBuffer& buffer, ClientID ID = 0);

    /**
    * This function can be used to send a string message to all the clients that are connected to this server.
    *
    * @params [in] message
    *       Bytes of data that needs to be sent.
    *
    * @param [in] clientToIgnore
    *       Optional param, ID of the client that we want to ignore sending the message to.
    *
    */
    void MessageAllClients(const std::string& message, ClientID ID = 0);

    /**
    * This function can be used to send a buffer to all the clients that are connected to this server.
    * The entire buffer will be sent.
    *
    * @params [in] buffer
    *       Bytes of data that needs to be sent.
    * 
    * @param [in] clientToIgnoreID
    *       Optional param, ID of the client that we want to ignore sending the buffer to.
    * 
    */
    void MessageAllClients(const std::vector<uint8_t>& buffer, ClientID ID = 0);

    /**
    * This function can be used to send a buffer to all the clients that are connected to this server.
    * You can restrict the number of bytes to be sent by using the 'numBytesToWrite' param.
    *
    * @params [in] buffer
    *       Bytes of data that needs to be sent.
    *
    * @params [in] numBytesToWrite
    *       Number of bytes of write from the 'buffer' vector.
    * 
    * @param [in] clientToIgnore
    *       Optional param, ID of the client that we want to ignore sending the buffer to.
    *
    */
    void MessageAllClients(const std::vector<uint8_t>& buffer, std::size_t numBytesToWrite, ClientID ID = 0);

    /**
    * Synchronous function to directly write string data to a socket.
    *
    * @params [in] socket
    *       Socket to write the data to
    *
    * @params [in] buffer
    *       String Data to be written to the socket.
    */
    void Write(boost::asio::ip::tcp::socket& socket, const std::string& buffer);

    /**
    * Synchronous function to write string data through a client handler pointer.
    *
    * @params [in] client
    *       ID of the client to write data to.
    *
    * @params [in] buffer
    *       String Data to be written to the socket.
    */
    void Write(ClientID ID, const std::string& buffer);

    /**
    * Synchronous function to write data through a client handler pointer.
    *
    * @params [in] client
    *       ID of the client to write data to.
    *
    * @params [in] buffer
    *       Bytes of data to be written to the socket.
    */
    void Write(ClientID ID, const std::vector<uint8_t>& buffer);

    /**
    * Synchronous function to write data through a client handler pointer.
    *
    * @params [in] client
    *       Pointer to the client handler to write data to.
    *
    * @params [in] buffer
    *       Bytes of data to be written to the socket.
    *
    * @params [in] numnumBytesToWrite
    *       Number of bytes of data to be written to the socket from the 'buffer'.
    */
    void Write(ClientID ID, const std::vector<uint8_t>& buffer, std::size_t numnumBytesToWrite);

    /**
    * Asynchronous function to write data to a Client.
    *
    * @params [in] ID
    *       ID of the client to write the data to.
    *
    * @params [in] buffer
    *       String Data to be written to the socket.
    */
    void AsyncWrite(ClientID ID, const std::string& buffer);

    /**
    * Asynchronous function to write data through a client handler pointer.
    *
    * @params [in] ID
    *       ID of the client to write the data to.
    *
    * @params [in] buffer
    *       Bytes of data to be written to the socket.
    */
    void AsyncWrite(ClientID ID, const std::vector<uint8_t>& buffer);

    /**
    * Asynchronous function to write data through a client handler pointer.
    *
    * @params [in] ID
    *       ID of the client to write the data to.
    *
    * @params [in] buffer
    *       Bytes of data to be written to the socket.
    *
    * @params [in] numBytesToWrite
    *       Number of bytes of data to be written to the socket from the 'buffer'.
    */
    void AsyncWrite(ClientID ID, const std::vector<uint8_t>& buffer, std::size_t numBytesToWrite);

    /**
    * This function makes the main thread wait, till boost::asio::io_context runs out of jobs to perform.
    */
    void Wait();

    /**
    * Return the number of clients currently connected to the server.
    */
    std::size_t GetNumClients() const { return m_ClientHandlers.size(); }

    /**
    * Returns the port number of the server.
    */
    int GetPort() const { return m_Port; }

protected:

    /**
    *   Returns if the input ID is valid or not.
    * 
    * @param [in] ID
    *       ID to be validated.
    */
    bool IsValidClientID(ClientID ID) const { return ID != 0; }

    /**
    * Returns the ClientHandler from the given ClientID.
    * 
    * @param [in] ID
    *       ID of the client whose handler is to be retreived.
    */
    const ClientHandlerSPtr GetClient(ClientID ID) const { return m_ClientHandlers.at(ID); }

private:

    /**
    * This function is called when the server starts, and adds the job of accepting a new connection to the io_context.
    */
    void WaitToAcceptNewConnection();

    /**
    * Helper function to return the io_context.
    */
    boost::asio::io_context& IOContext() { return m_IOContext; }

private:

    /* Port that the server is listening on. */
    int                                     m_Port;

    /* IO context of the server. */
    boost::asio::io_context                 m_IOContext;

    /* Thread on which the io_context performs it's tasks */
    std::thread                             m_ContextThread;

    /* Used to accept new client connections to the server. */
    boost::asio::ip::tcp::acceptor          m_Acceptor;

    /* Mutext to protect the m_Clients. */
    std::mutex                              m_MutexClients;

    /* Map of ClientID against the Client Handler pointer. */
    std::unordered_map<ClientID, std::shared_ptr<ClientHandler>>   m_ClientHandlers;

    /* ID that will be assigned to the next client that will connect to the server. */
    uint32_t                                m_NewClientID;

    /* Maximum number of clients that are allowed to connect to the server. */
    uint32_t                                m_MaxClientsAllowed;
};

END_NAMESPACE_TCP
