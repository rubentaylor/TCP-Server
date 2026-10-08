#pragma once
#include "TCPListener.h"
#include <mutex>
#include <unordered_map>
#include <string>

class ClientChat : public TCPListener
{
public:
    ClientChat(const char *ipAdress, int port) : TCPListener(ipAdress, port) {}

protected:
    void onConnect(int clientSock) override;
    void onDisconnect(int clientSock) override;
    void onRecievedMessage(int clientSock, const char *msg, int msgLength) override;

private:
    // Store the mapping of client sockets to usernames
    std::unordered_map<int, std::string> _clientUsernames;
    std::mutex _usernamesMutex; // Multiple client threads access usernames concurrently.
};