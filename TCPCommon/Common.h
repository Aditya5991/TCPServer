#pragma once

#include <string>
#include <vector>
#include <functional>
#include <thread>
#include <mutex>
#include <inttypes.h>

#define CRLF "\r\n"

#define BEGIN_NAMESPACE_NET namespace net {
#define END_NAMESPACE_NET   }

#define BEGIN_NAMESPACE_TCP     BEGIN_NAMESPACE_NET namespace tcp {
#define END_NAMESPACE_TCP       END_NAMESPACE_NET   }

BEGIN_NAMESPACE_TCP

using ClientID = std::size_t;

class Server;
using ServerSPtr = std::shared_ptr<Server>;

class ClientHandler;
using ClientHandlerSPtr = std::shared_ptr<ClientHandler>;


END_NAMESPACE_TCP
