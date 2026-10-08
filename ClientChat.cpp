#include "ClientChat.h"
#include "TCPListener.h"
#include <string>
#include <iostream>
#include <algorithm>

std::string trimMessage(const std::string &message)
{
    std::string trimmed = message;
    while (!trimmed.empty() && (trimmed.back() == '\r' || trimmed.back() == '\n'))
    {
        trimmed.pop_back();
    }
    while (!trimmed.empty() && (trimmed.front() == ' ' || trimmed.front() == '\t'))
    {
        trimmed.erase(trimmed.begin());
    }
    while (!trimmed.empty() && (trimmed.back() == ' ' || trimmed.back() == '\t'))
    {
        trimmed.pop_back();
    }
    return trimmed;
}

void ClientChat::onConnect(int clientSock)
{
    std::string welcomeMessage = "Welcome to the server! Please enter your username: ";
    clientBroadcast(clientSock, welcomeMessage.c_str(), static_cast<int>(welcomeMessage.size()));
}

void ClientChat::onDisconnect(int clientSock)
{
    std::lock_guard<std::mutex> lock(_usernamesMutex);
    _clientUsernames.erase(clientSock);
}

void ClientChat::onRecievedMessage(int clientSock, const char *msg, int msgLength)
{
    std::string incomingText(msg, msgLength);
    std::string cleanedText = trimMessage(incomingText);

    if (cleanedText.empty())
    {
        return;
    }

    std::string outgoingMessage;
    {
        std::lock_guard<std::mutex> lock(_usernamesMutex);
        auto userIt = _clientUsernames.find(clientSock);
        if (userIt == _clientUsernames.end())
        {
            _clientUsernames[clientSock] = cleanedText;
            outgoingMessage = cleanedText + " has joined the chat.\n";
        }
        else
        {
            outgoingMessage = userIt->second + ": " + cleanedText + "\n";
        }
    }

    globalBroadcast(clientSock, outgoingMessage.c_str(), static_cast<int>(outgoingMessage.size()));
}
