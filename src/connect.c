#include "connect.h"

#include <stdint.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include <arpa/inet.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>


int32_t opensocket(Socket *newsocket, uint16_t port, uint8_t connection_amnt)
{
	newsocket->socket = socket(AF_INET, SOCK_STREAM, 0);
	if (newsocket->socket == -1)
	{
		perror("Error openning socket");
		return 1;
	}

	newsocket->opt = 0;

	int32_t options = SO_REUSEADDR | SO_REUSEPORT | SO_KEEPALIVE; 
	int32_t optres = setsockopt(newsocket->socket, SOL_SOCKET, options, &newsocket->opt, sizeof(newsocket->opt));
	if (optres != 0)
	{
		perror("couldnt setsockopt");
		return 1;
	}

	newsocket->addr.sin_port = htons(port);
	newsocket->addr.sin_addr.s_addr = INADDR_ANY;
	newsocket->addr.sin_family = AF_INET;

	if (bind(newsocket->socket, (struct sockaddr *)&newsocket->addr, sizeof(newsocket->addr) ) == -1)
	{
		perror("Error binding address");
		shutdown(newsocket->socket, SHUT_RDWR);
		return 1;
	}

	//printf("Successfully bound to port %u\n", port);

	if (listen(newsocket->socket, connection_amnt) < 0)
	{
		shutdown(newsocket->socket, SHUT_RDWR);
		return 1;
	}

	return 0;
}


int32_t connecttoapache(uint32_t port, Socket *redirect)
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
		shutdown(redirect->socket, SHUT_RDWR);
		return 1;
	}

	return 0;
}


int32_t connectclient(int32_t proxysocket)
{
	int32_t clientsocket = accept(proxysocket, NULL, NULL);
	if (clientsocket == -1)
	{
		perror("Error connecting to client");
		return -1;
	}

	return clientsocket;
}

