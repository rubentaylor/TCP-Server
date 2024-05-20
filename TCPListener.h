#pragma once 
#include <iostream>
#include <WS2tcpip.h> //  WinSock2 Extension for TCP/IP protocols
#include <Winsock2.h>
#include <string>
#include <sstream>

#pragma comment (lib, "ws2_32.lib")

class TCPListener {

public:
    TCPListener(const char* ipAddress, int port) 
    :_ipAddress(ipAddress),_port(port) {}
    int initializer();
    int run();
protected:
    virtual void onConnect(int clientSock);
    virtual void onDisconnect(int clientSock);
    virtual void onRecievedMessage(int clientSock, const char* msg, int msgLength);
    void clientBroadcast(int clientSock, const char* msg, int msgLength);
    void globalBroadcast(int whoSent, const char* msg, int msgLength);
private:
    //Internals 
    int _socket;
    const char* _ipAddress;
    int _port;
    fd_set _master;
};