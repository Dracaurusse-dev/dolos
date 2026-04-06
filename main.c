#include <stdio.h>
#include <stdint.h>
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
	int32_t socket, opt;
} Socket;


int opensocket(Socket *newsocket)
{
	newsocket->socket = socket(AF_INET, SOCK_STREAM, 0);
	if (newsocket->socket == -1)
	{
		perror("Error openning socket");
		return 1;
	}

	int32_t options = SO_REUSEADDR | SO_REUSEPORT | SO_KEEPALIVE; 
	int32_t optres = setsockopt(newsocket->socket, SOL_SOCKET, options, &newsocket->opt, sizeof(newsocket->opt));
	if (optres != 0)
	{
		perror("couldnt setsockopt");
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
	if (redirect->socket == -1)
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

void closesockets(int32_t *sockets, int32_t socketcount)
{
	for (int32_t i = 0; i < socketcount; i++)
	{
		close(sockets[i]);
	}
}


int main(void)
{
	int32_t clientsocket;
	Socket serversocket, redirectsocket;
	if (opensocket(&serversocket) == 1)
		return 1;	

	if (listen(serversocket.socket, 5) != 0)
		return 1;
		
	clientsocket = accept(serversocket.socket, NULL, NULL);
	if (clientsocket == -1)
	{
		perror("Error connecting to client");
		closesockets((int []){serversocket.socket, clientsocket, redirectsocket.socket}, 3);
		return 1;
	}

	// Establish a connection to apache
	int32_t res = connecttoapache(8000, &redirectsocket);
	if (res != 0)
	{
		perror("Couldnt connect to apache");
		closesockets((int []){serversocket.socket, redirectsocket.socket}, 2);
		return 1;
	}



	int recvres = 0;
	while (recvres != -1)
	{
		
		// Read the request from the client
		char buffer[8192] = {0};
		recvres = recv(clientsocket, buffer, sizeof(buffer), 0);
		if (recvres == 0)
			continue;
		printf("message:\n%s\n", buffer);

		// Send the packets back to the client
		send(redirectsocket.socket, buffer, strlen(buffer), 0);
		recv(redirectsocket.socket, buffer, sizeof(buffer), 0);
		printf("messga from redirect:\n%s\n", buffer);
		send(clientsocket, buffer, strlen(buffer), 0);
		
	}

	closesockets((int []){serversocket.socket, clientsocket, redirectsocket.socket}, 3);

	return 0;
}

