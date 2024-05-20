#include "TCPListener.h"
int TCPListener::initializer(){
    
	// Initialze windows socket
	WSADATA wsData;
	WORD ver = MAKEWORD(2, 2);

	int wsOk = WSAStartup(ver, &wsData);
	if (wsOk != 0)
	{
		std::cerr << "Can't initialize Socket" << std::endl;
		return wsOk;
	}
	
	// Create a socket
	_socket = socket(AF_INET, SOCK_STREAM, 0);
	if (_socket == INVALID_SOCKET)
	{
		return WSAGetLastError();
	}

    // Bind the ip and port to a socket
	sockaddr_in hint;
	hint.sin_family = AF_INET; // IPv4
	hint.sin_port = htons(_port); // Converts port number to network byte 
	inet_pton(AF_INET, _ipAddress, &hint.sin_addr); // IP to binary
	
	
	if (bind(_socket, (sockaddr*)&hint, sizeof(hint)) == SOCKET_ERROR){
        return WSAGetLastError();
    }

	if (listen(_socket, SOMAXCONN) == SOCKET_ERROR){
		return WSAGetLastError();
	}
	FD_ZERO(&_master);
	FD_SET(_socket, &_master);
	return 0;

}

int TCPListener::run(){

	bool running = true;
	while (running)
	{
		fd_set copy = _master;

		// how many clients are conntected
		int socketCount = select(0, &copy, nullptr, nullptr, nullptr);

		// Loop current connections
		for (int i = 0; i < socketCount; i++)
		{
			SOCKET sock = copy.fd_array[i];
			if (sock == _socket)
			{
				// Accept a new connection
				SOCKET client = accept(_socket, nullptr, nullptr);
				// Add the new connection to the list of connected clients
				FD_SET(client, &_master);
				onConnect(client);
			}
			else // It's an inbound message
			{
				char buf[4096];
				ZeroMemory(buf, 4096);
				
				// Receive message
				int bytesIn = recv(sock, buf, 4096, 0);

				if (bytesIn <= 0) // client has dropped
				{
					onDisconnect(sock);
					closesocket(sock);
					FD_CLR(sock, &_master);
				}
				else
				{
					onRecievedMessage(sock, buf, bytesIn);
					
				}
			}
		}
	}

	//Close connection
	FD_CLR(_socket,&_master);
	closesocket(_socket);

	std::string closing = "Server is now closing. \r\n";
	while (_master.fd_count > 0){
		SOCKET sock = _master.fd_array[0];
		//send(sock,closing.c_str(),closing.size()+1,0);
		FD_CLR(sock,&_master);
		closesocket(sock);
	}
	WSACleanup();
	return 0;
}
void TCPListener::clientBroadcast(int clientSock, const char* msg, int msgLength){
	send(clientSock, msg, msgLength, 0);
}
void TCPListener::globalBroadcast(int whoSent, const char* msg, int msgLength){
    for (int i = 0; i < _master.fd_count; i++){
        SOCKET outSock = _master.fd_array[i];
        if (outSock != _socket && outSock != whoSent)
        {
            clientBroadcast(outSock, msg, msgLength);
        }
    }
}
void TCPListener::onConnect(int clientSock){
	
}
void TCPListener::onDisconnect(int clientSock){

}
void TCPListener::onRecievedMessage(int clientSock, const char* msg, int msgLength){

}