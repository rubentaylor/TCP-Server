#include "ClientChat.h"
#include  "TCPListener.h"
#include <string>

void ClientChat::onConnect(int clientSock){
    // Welcome
    std::string welcome = "Welcome to the server!";
    send(clientSock, welcome.c_str(), welcome.size()+1,0);
}
void ClientChat::onDisconnect(int clientSock){

}
void ClientChat::onRecievedMessage(int clientSock, const char* msg, int msgLength){
    TCPListener::clientBroadcast(clientSock, msg, msgLength);
}
