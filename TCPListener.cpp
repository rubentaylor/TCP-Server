#include "TCPListener.h"
#include <algorithm>
#include <cstring>

namespace tcp
{
#ifdef _WIN32
	void closeSocket(SOCKET socketValue)
	{
		::closesocket(socketValue);
	}
#else
	void closeSocket(SOCKET socketValue)
	{
		::close(socketValue);
	}
#endif
}
#ifdef _WIN32
#define INVALID INVALID_SOCKET
#else
#define INVALID -1
#endif

int TCPListener::initializer()
{
#ifdef _WIN32
	WSADATA socketData;
	WORD version = MAKEWORD(2, 2);

	int startupResult = WSAStartup(version, &socketData);
	if (startupResult != 0)
	{
		std::cerr << "Can't initialize Socket" << std::endl;
		return startupResult;
	}
#endif

	// Create socket
	_socket = socket(AF_INET, SOCK_STREAM, 0);
	if (_socket == INVALID)
	{
#ifdef _WIN32
		return WSAGetLastError();
#else
		std::cerr << "socket() failed" << std::endl;
		return 1;
#endif
	}

	// Fill in the socket address structure
	sockaddr_in addressInfo{};
	addressInfo.sin_family = AF_INET; // ipv4
	addressInfo.sin_port = htons(_port);
	inet_pton(AF_INET, _ipAddress, &addressInfo.sin_addr);

	// Bind the socket to the  port
	if (bind(_socket, reinterpret_cast<sockaddr *>(&addressInfo), sizeof(addressInfo)) == INVALID)
	{
		std::cerr << "bind() failed" << std::endl;
		return 1;
	}

	// Start listening for incoming connections
	if (listen(_socket, SOMAXCONN) == INVALID)
	{
		std::cerr << "listen() failed" << std::endl;
		return 1;
	}
	std::cout << "Server listening on " << _ipAddress << ":" << _port << std::endl;
	return 0;
}

void TCPListener::acceptNewClient()
{
	SOCKET clientSocket = accept(_socket, nullptr, nullptr);
	if (clientSocket == INVALID)
	{
		return;
	}

	// only locks the mutex when adding the socket to the list, destroys lock after
	{
		std::lock_guard<std::mutex> lock(_clientSocketsMutex);
		_clientSockets.push_back(clientSocket);
	}

	onConnect(clientSocket);
	int threadCount = currentThreadCount.fetch_add(1) + 1;
	// Start a thread to handle this client.
	std::thread(&TCPListener::handleClient, this, clientSocket).detach();
	std::cout << "New client connected from: " << _ipAddress << ". Threads:" << threadCount << std::endl;
}

void TCPListener::handleClient(SOCKET clientSocket)
{
	char buffer[4096];
	while (true)
	{
		int bytesReceived = recv(clientSocket, buffer, sizeof(buffer) - 1, 0);
		if (bytesReceived <= 0)
		{
			disconnectClient(clientSocket);
			break;
		}

		onRecievedMessage(clientSocket, buffer, bytesReceived);
	}

	int threadCount = currentThreadCount.fetch_sub(1) - 1;
	std::cout << "Client thread ended. Threads:" << threadCount << std::endl;
}

void TCPListener::disconnectClient(SOCKET clientSocket)
{
	onDisconnect(clientSocket);
	{
		std::lock_guard<std::mutex> lock(_clientSocketsMutex);
		_clientSockets.erase(
			std::remove(_clientSockets.begin(), _clientSockets.end(), clientSocket),
			_clientSockets.end());
		tcp::closeSocket(clientSocket);
	}
}

int TCPListener::run()
{
	while (true)
	{
		acceptNewClient();
	}
}

void TCPListener::onConnect(int clientSock)
{
}

void TCPListener::onDisconnect(int clientSock)
{
}

// Sends one message to a single connected client.
void TCPListener::clientBroadcast(int clientSock, const char *msg, int msgLength)
{
	std::lock_guard<std::mutex> lock(_clientSocketsMutex);
	sendMessage(clientSock, msg, msgLength);
}

void TCPListener::onRecievedMessage(int clientSock, const char *msg, int msgLength)
{
	globalBroadcast(clientSock, msg, msgLength);
}

void TCPListener::globalBroadcast(int whoSent, const char *msg, int msgLength)
{
	std::lock_guard<std::mutex> lock(_clientSocketsMutex);
	for (SOCKET clientSocket : _clientSockets)
	{
		if (clientSocket != whoSent)
		{
			sendMessage(clientSocket, msg, msgLength);
		}
	}
}

void TCPListener::sendMessage(SOCKET clientSocket, const char *msg, int msgLength)
{
	send(clientSocket, msg, msgLength, 0);
}