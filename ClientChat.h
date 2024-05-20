//#pragma once
#include "TCPListener.h"
class ClientChat : public TCPListener{

public:
    ClientChat(const char* ipAdress, int port): TCPListener(ipAdress, port){}
protected:
    void onConnect(int clientSock);
    void onDisconnect(int clientSock);
    void onRecievedMessage(int clientSock, const char* msg, int msgLength);
    void clientBroadcast(int clientSock, const char* msg, int msgLength);
    void globalBroadcast(int whoSent, const char* msg, int msgLength);
};