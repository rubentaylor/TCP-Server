#include "ClientChat.h"
#include  "TCPListener.h"
#include <string>

void ClientChat::onConnect(int clientSock){
    // Welcome
    std::string welcome = "Welcome to the server!";
    clientBroadcast(clientSock, welcome.c_str(), welcome.size()+1);
}
void ClientChat::onDisconnect(int clientSock){

}
void ClientChat::onRecievedMessage(int clientSock, const char* msg, int msgLength){
    globalBroadcast(clientSock, msg, msgLength);
}
