#include <iostream>
#include <thread>
#include <boost/array.hpp>
#include <boost/asio.hpp>
#include <boost/bind/bind.hpp>

#include "SMTPTestClient.h"
#include "SimpleTCPClient.h"

int testSMTPClient()
{
    SMTPClient client;
    client.AsyncConnect("smtp.gmail.com", 587);
    client.Wait();
    return 0;
}

int testSimpleTCPClient()
{
    SimpleTCPClient client;
    client.AsyncConnect("127.0.0.1", 65520);
    client.Wait();
    return 0;
}

int main(int argc, char* argv[])
{
    return testSMTPClient();
    return testSimpleTCPClient();
}