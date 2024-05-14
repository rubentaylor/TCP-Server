#pragma once 

#include <iostream>
#include <WS2tcpip.h> //  WinSock2 Extension for TCP/IP protocols
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

private:
    //Internals 
    int _socket;
    const char* _ipAddress;
    int _port;
}