#pragma once

#include "Common.h"
#include <boost/asio.hpp>
#include <thread>
#include <mutex>
#include <queue>
#include <inttypes.h>


BEGIN_NAMESPACE_TCP

using boost::asio::ip::tcp;

class Client;

class Server
{
public:

    Server(int port, uint32_t maxClientsAllowed = -1);
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
    virtual bool OnClientConnected(const Client* client) = 0;

    /**
    * This function is a callback which is called when data is received for any client.
    * 
    * @params [in] client
    *       const Pointer to client handler class that has just connected to this server.
    * 
    */
    virtual void OnDataReceived(const Client* client) = 0;

    /**
    * This function is called when any errorneous data is received from any client.
    * For now, this function disconnects the client directly.
    * 
    * @params [in] client
    *       const ptr to the client handler from which the errorneous data is received.
    * 
    * @params [in] ec
    *       error code received from boost::asio.
    */
    void OnDataReceivedError(const Client* client, const boost::system::error_code& ec);
    
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
    bool OnClientConnected(tcp::socket socket);

    /**
    * This function is called when a client disconnects from the server.
    * Removes the client from the clients map.
    * 
    * @params [in] client
    *       Pointer to the client handler object that disconnected from the server.
    */
    void OnDisconnect(const Client* client);

    /**
    * This function can be used to send a message to all the clients that are connected to this server.
    *
    * @params [in] message
    *       Bytes of data that needs to be sent.
    * 
    * @param [in] clientToIgnore
    *       Optional param, used to ignore any particular client.
    * 
    */
    void MessageAllClients(const std::vector<uint8_t>& message, const Client* clientToIgnore = nullptr);

    /**
    * This function can be used to send a message to all the clients that are connected to this server.
    *
    * @params [in] message
    *       Bytes of data that needs to be sent.
    *
    * @params [in] bytesToWrite
    *       Number of bytes of write from the 'message' vector.
    * 
    * @param [in] clientToIgnore
    *       Optional param, used to ignore any particular client.
    *
    */
    void MessageAllClients(const std::vector<uint8_t>& message, std::size_t bytesToWrite, const Client* clientToIgnore = nullptr);

    /**
    * Synchronous function to directly write data to a socket.
    *
    * @params [in] socket
    *       Socket to write the data to
    *
    * @params [in] buffer
    *       String Data to be written to the socket.
    */
    void Write(tcp::socket& socket, const std::string& buffer);

    /**
    * Synchronous function to write data through a client handler pointer.
    *
    * @params [in] client
    *       Pointer to the client handler to write data to.
    *
    * @params [in] buffer
    *       String Data to be written to the socket.
    */
    void Write(Client* client, const std::string& buffer);

    /**
    * Synchronous function to write data through a client handler pointer.
    *
    * @params [in] client
    *       Pointer to the client handler to write data to.
    *
    * @params [in] buffer
    *       Bytes of data to be written to the socket.
    */
    void Write(Client* client, const std::vector<uint8_t>& buffer);

    /**
    * Synchronous function to write data through a client handler pointer.
    *
    * @params [in] client
    *       Pointer to the client handler to write data to.
    *
    * @params [in] buffer
    *       Bytes of data to be written to the socket.
    *
    * @params [in] numBytesToWrite
    *       Number of bytes of data to be written to the socket from the 'buffer'.
    */
    void Write(Client* client, const std::vector<uint8_t>& buffer, std::size_t numBytesToWrite);

    /**
    * Asynchronous function to write data through a client handler pointer.
    *
    * @params [in] client
    *       Pointer to the client handler to write data to.
    *
    * @params [in] buffer
    *       String Data to be written to the socket.
    */
    void AsyncWrite(Client* client, const std::string& buffer);

    /**
    * Synchronous function to write data through a client handler pointer.
    *
    * @params [in] client
    *       Pointer to the client handler to write data to.
    *
    * @params [in] buffer
    *       Bytes of data to be written to the socket.
    */
    void AsyncWrite(Client* client, const std::vector<uint8_t>& buffer);

    /**
    * Synchronous function to write data through a client handler pointer.
    *
    * @params [in] client
    *       Pointer to the client handler to write data to.
    *
    * @params [in] buffer
    *       Bytes of data to be written to the socket.
    *
    * @params [in] numBytesToWrite
    *       Number of bytes of data to be written to the socket from the 'buffer'.
    */
    void AsyncWrite(Client* client, const std::vector<uint8_t>& buffer, std::size_t numBytesToWrite);

    /**
    * This function makes the main thread wait, till boost::asio::io_context runs out of jobs to perform.
    */
    void Wait();

    /**
    * Return the number of clients currently connected to the server.
    */
    std::size_t GetNumClients() const { return m_Clients.size(); }

    /**
    * Returns the port number of the server.
    */
    int GetPort() const { return m_Port; }

private:

    /**
    * This dunction is called when the server starts, and adds the job of accepting a new connection to the io_context.
    */
    void WaitToAcceptNewConnection();

    /**
    * Helper function to return the io_context.
    */
    boost::asio::io_context& IOContext() { return m_IOContext; }

private:

    /* Port that the server is listening on. */
    int                                 m_Port;

    /* IO context of the server. */
    boost::asio::io_context             m_IOContext;

    /* Thread on which the io_context performs it's tasks */
    std::thread                         m_ContextThread;

    /* Used to accept new client connections to the server. */
    boost::asio::ip::tcp::acceptor      m_Acceptor;

    /* Mutext to protect the m_Clients. */
    std::mutex                          m_MutexClients;

    /* Map of ClientID against the Client Handler pointer. */
    std::unordered_map<int, Client*>    m_Clients;

    /* ID that will be assigned to the next client that will connect to the server. */
    uint32_t                            m_NewClientID;

    /* Maximum number of clients that are allowed to connect to the server. */
    uint32_t                            m_MaxClientsAllowed;
};

END_NAMESPACE_TCP
