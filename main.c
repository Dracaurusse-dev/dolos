#include <stdio.h>
#include <stdint.h>
#include <strings.h>
#include <string.h>
#include <stdlib.h>

#include <unistd.h>

#include <sys/socket.h>
#include <netinet/in.h>
#include <sys/types.h>
#include <netdb.h>
#include <arpa/inet.h>


#define PORT 8080


typedef struct
{
	struct sockaddr_in addr;
	int32_t socket;
} Socket;


int opensocket(Socket *newsocket)
{
	newsocket->socket = socket(AF_INET, SOCK_STREAM, 0);
	if (newsocket->socket == -1)
	{
		perror("Error openning socket");
		return 1;
	}

	newsocket->addr.sin_port = htons(PORT);
	newsocket->addr.sin_addr.s_addr = INADDR_ANY;
	newsocket->addr.sin_family = AF_INET;

	if (bind(newsocket->socket, (struct sockaddr *)&newsocket->addr, sizeof(struct sockaddr_in) ) == -1)
	{
		perror("Error binding address");
		return 1;
	}

	printf("Successfully bound to port %u\n", PORT);

	return 0;
}


int connecttoapache(uint32_t port, Socket *redirect)
{
	redirect->socket = socket(AF_INET, SOCK_STREAM, 0);
	if (redirect->socket  == -1)
	{
		perror("Couldnt create redirect socket");
		return 1;
	}

	bzero(&(redirect->addr), sizeof(redirect->addr));
	redirect->addr.sin_family = AF_INET;
	redirect->addr.sin_addr.s_addr = inet_addr("127.0.0.1"); 
	redirect->addr.sin_port = htons(port);

	if (connect(redirect->socket, (struct sockaddr *)&redirect->addr, sizeof(redirect->addr)) != 0)
	{
		perror("Connection failed");
		return 1;
	}

	return 0;
}


int main(void)
{
	Socket serversocket;
	if (opensocket(&serversocket) == 1)
		return 1;	

	listen(serversocket.socket, 5);

	int clientsocket = accept(serversocket.socket, NULL, NULL);
	
	// Read the first request
	char buffer[1024] = {0};
	recv(clientsocket, buffer, sizeof(buffer), 0);
	printf("Message %s\n", buffer);
	
	// Establish a connection to apache
	Socket redirectsocket;
	connecttoapache(80, &redirectsocket);

	send(redirectsocket.socket, buffer, strlen(buffer), 0);
	recv(redirectsocket.socket, buffer, sizeof(buffer), 0);
	send(clientsocket, buffer, strlen(buffer), 0);

	close(clientsocket);
	close(redirectsocket.socket);
	close(serversocket.socket);

	return 0;
}

