#include "TCPListener.h"

int TCPListener::initializer(){
    
	// Initialze windows socket
	WSADATA wsData;
	WORD ver = MAKEWORD(2, 2);

	int wsOk = WSAStartup(ver, &wsData);
	if (wsOk != 0)
	{
		std::cerr << "Can't initialize Socket" << std::endl;
		return;
	}
	
	// Create a socket
	_socket= socket(AF_INET, SOCK_STREAM, 0);
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

}

int TCPListener::run(){

}