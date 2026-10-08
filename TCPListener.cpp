#include "TCPListener.h"

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
#define INVALID -1

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
	sockaddr_in addressInfo;
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
	// Track the listening socket
	FD_ZERO(&_trackedSockets);		   // clear the master set
	FD_SET(_socket, &_trackedSockets); // add the listening socket to the master set
	return 0;
}

void TCPListener::acceptNewClient()
{
	SOCKET clientSocket = accept(_socket, nullptr, nullptr);
	if (clientSocket == INVALID)
	{
		return;
	}

	FD_SET(clientSocket, &_trackedSockets); // add the new client socket to the master set
	_clientSockets.push_back(clientSocket);
	onConnect(clientSocket);
}

void TCPListener::processClientMessage(SOCKET clientSocket)
{
	char buffer[4096];
	std::memset(buffer, 0, sizeof(buffer));

	int bytesReceived = recv(clientSocket, buffer, sizeof(buffer) - 1, 0);
	if (bytesReceived <= 0)
	{
		disconnectClient(clientSocket);
		return;
	}

	onRecievedMessage(clientSocket, buffer, bytesReceived);
}

void TCPListener::disconnectClient(SOCKET clientSocket)
{
	onDisconnect(clientSocket);
	tcp::closeSocket(clientSocket);
	FD_CLR(clientSocket, &_trackedSockets);
	_clientSockets.erase(
		std::remove(_clientSockets.begin(), _clientSockets.end(), clientSocket),
		_clientSockets.end());
}

void TCPListener::handleReadySocket(SOCKET socket)
{
	if (socket == _socket)
	{
		acceptNewClient();
		return;
	}

	processClientMessage(socket);
}

int TCPListener::run()
{
	bool running = true;

	while (running)
	{
		fd_set readySockets = _trackedSockets;
		int socketCount = select(FD_SETSIZE, &readySockets, nullptr, nullptr, nullptr);

		if (socketCount <= 0)
		{
			continue;
		}

#ifdef _WIN32
		for (int i = 0; i < socketCount; ++i)
		{
			handleReadySocket(readySockets.fd_array[i]);
		}
#else
		for (int fileDescriptor = 0; fileDescriptor < FD_SETSIZE; ++fileDescriptor)
		{
			if (FD_ISSET(fileDescriptor, &readySockets))
			{
				handleReadySocket(fileDescriptor);
			}
		}
#endif
	}

	// Close everything before the server stops
	FD_CLR(_socket, &_trackedSockets);
	tcp::closeSocket(_socket);

	for (SOCKET clientSocket : _clientSockets)
	{
		FD_CLR(clientSocket, &_trackedSockets);
		tcp::closeSocket(clientSocket);
	}
	_clientSockets.clear();

#ifdef _WIN32
	WSACleanup();
#endif
	return 0;
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
	send(clientSock, msg, msgLength, 0);
}

void TCPListener::onRecievedMessage(int clientSock, const char *msg, int msgLength)
{
	globalBroadcast(clientSock, msg, msgLength);
}

void TCPListener::globalBroadcast(int whoSent, const char *msg, int msgLength)
{
	// Send message to all other clients except themselves
	for (SOCKET clientSocket : _clientSockets)
	{
		if (clientSocket != _socket && clientSocket != whoSent)
		{
			clientBroadcast(clientSocket, msg, msgLength);
		}
	}
}