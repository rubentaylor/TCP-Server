#pragma once

#include <atomic>
#include <iostream>
#include <mutex>
#include <vector>
#include <thread>

#ifdef _WIN32
#include <winsock2.h>
#include <ws2tcpip.h>
#pragma comment(lib, "ws2_32.lib")
#else
#include <arpa/inet.h>
#include <netinet/in.h>
#include <sys/select.h>
#include <sys/socket.h>
#include <unistd.h>
#include <cstring>
using SOCKET = int;
#endif

class TCPListener
{
public:
    // Store the bind address and port; values are used by initializer()
    TCPListener(const char *ipAddress, int port)
    {
        _ipAddress = ipAddress;
        _port = port;
    }

    // Creates the server socket and puts it in listening mode.
    int initializer();

    // the main accept and read loop
    int run();

protected:
    virtual void onConnect(int clientSock);
    virtual void onDisconnect(int clientSock);
    virtual void onRecievedMessage(int clientSock, const char *msg, int msgLength);
    void clientBroadcast(int clientSock, const char *msg, int msgLength);
    void globalBroadcast(int whoSent, const char *msg, int msgLength);

private:
    void acceptNewClient();
    void handleClient(SOCKET clientSocket);
    void disconnectClient(SOCKET clientSocket);
    void sendMessage(SOCKET clientSocket, const char *msg, int msgLength);

    SOCKET _socket;                         // listening socket for incoming connections
    const char *_ipAddress;                 // address the server binds to
    int _port;                              // port the server listens on
    std::vector<SOCKET> _clientSockets;     // connected client sockets
    std::mutex _clientSocketsMutex;         // protects client list and sends
    std::atomic<int> currentThreadCount{0}; // tracks active client threads
};