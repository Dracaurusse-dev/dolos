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
#define WEBSITE_PORT 8000
#define BUFFER_SIZE 1046576
#define MAX_SOCKET_CONN 5


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

	if (bind(newsocket->socket, (struct sockaddr *)&newsocket->addr, sizeof(newsocket->addr) ) == -1)
	{
		perror("Error binding address");
		close(newsocket->socket);
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
		close(redirect->socket);
		return 1;
	}

	return 0;
}


void closesockets(int32_t *sockets, int32_t socketcount)
{
	for (int32_t i = 0; i < socketcount; i++)
	{
		int32_t r = close(sockets[i]);
		if (r != 0)
			fprintf(stderr, "socket %d couldnt close", i);
	}
}


int main(void)
{
	char buffer[BUFFER_SIZE];
	int32_t clientsocket, errorcode;
	Socket proxysocket, redirectsocket;
	if (opensocket(&proxysocket) == 1)
		return 1;	

	if (listen(proxysocket.socket, MAX_SOCKET_CONN) < 0)
	{
		close(proxysocket.socket);
		return 1;
	}

	while (1)
	{
		clientsocket = accept(proxysocket.socket, NULL, NULL);
		if (clientsocket == -1)
		{
			perror("Error connecting to client");
			errorcode=1;
			close(clientsocket);
			break;
		}

		// Establish a connection to apache
		int32_t res = connecttoapache(WEBSITE_PORT, &redirectsocket);
		if (res != 0)
		{
			perror("Couldnt connect to apache");
			errorcode=1;
			close(clientsocket);
			close(redirectsocket.socket);
			break;
		}

		// Read the request from the client
		int32_t bytes_read = recv(clientsocket, buffer, BUFFER_SIZE, 0);
		if (bytes_read == 0)
			continue;
		if (bytes_read == -1)
		{
			perror("Couldnt receive client's request");
			errorcode=1;
			break;
		}
		if (bytes_read == BUFFER_SIZE)
			printf("alot of bytes");
		printf("message from client:\n%s\n", buffer);

		// Send the packet to the server
		int32_t sendres = send(redirectsocket.socket, buffer, bytes_read, 0);
		if (sendres == 0)
			continue;
		if (sendres == -1)
		{
			perror("Couldnt send to server with redirectsocket");
			errorcode=1;
			break;
		}

		// Receive the reply and send it to the client
		bytes_read = recv(redirectsocket.socket, buffer, sizeof(buffer), 0);
		if (bytes_read == 0)
			continue;
		if (bytes_read == -1)
		{
			perror("Couldnt receive server's answer");
			errorcode=1;
			break;
		}
		if (bytes_read == BUFFER_SIZE)
			printf("alot of bytes");
		printf("message from redirect:\n%s\n", buffer);

		send(clientsocket, buffer, bytes_read, 0);
		if (sendres == 0)
			continue;
		if (sendres == -1)
		{
			perror("Couldnt send to client with clientsocket");
			errorcode=1;
			break;
		}

		close(clientsocket);
		close(redirectsocket.socket);
		
	}
	
	closesockets((int []){proxysocket.socket, clientsocket, redirectsocket.socket}, 3);

	return errorcode;
}

