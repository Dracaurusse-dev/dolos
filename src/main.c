#include "connect.h"

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
#define BUFFER_SIZE 4096
#define MAX_SOCKET_CONN 5


char *longrecv(int32_t socket, ssize_t *lengthoutput)
{
	ssize_t length = BUFFER_SIZE;
	ssize_t res;
	char *buf = (char *)malloc(length);
	*lengthoutput=0;
	uint16_t incrementamnt = 1024;
	
	do 
	{
		res = recv(socket, buf, length, MSG_PEEK);
		if (res == -1)
		{
			perror("couldnt read the message");
			return NULL;
		}

		length += incrementamnt;
		buf = realloc(buf, length);

		if (buf == NULL)
		{
			perror("Couldn't realloc buf");
			return NULL;
		}
	}
	while (res >= length-incrementamnt);

	buf = realloc(buf, length);
	*lengthoutput = recv(socket, buf, length, 0);

	printf("lengthoutput %ld\n", *lengthoutput);
	return buf;
}


int main(void)
{
	int32_t clientsocket;
	Socket proxysocket, redirectsocket;

	int32_t openres = opensocket(&proxysocket, PORT);	
	if (openres == 1)
	{
		perror("Couldn't open the socket");
		return 1;
	}

	while (1)
	{
		clientsocket = connectclient();
		if (clientsocket == -1)
		{
			break;
		}

		int32_t res = connecttoapache(WEBSITE_PORT, &redirectsocket);
		if (res != 0)
		{
			perror("Couldnt connect to apache");
			clean(proxysocket.socket, redirectsocket.socket, clientsocket, NULL);
			return 1;
		}

		// Read the request from the client	
		ssize_t buflen;
		char *buf = longrecv(clientsocket, &buflen);

		if (buf == NULL)
		{
			clean(proxysocket.socket, redirectsocket.socket, clientsocket, buf);
			return 1;
		}

		printf("client message: \n%s\n", buf);

		// Send the packet to the server
		int32_t sendres = send(redirectsocket.socket, buf, buflen, 0);
		if (sendres == -1)
		{
			perror("Couldnt send to server with redirectsocket");
			clean(proxysocket.socket, redirectsocket.socket, clientsocket, buf);
			return 1;
		}

		// Receive the reply and send it to the client
		buf = longrecv(redirectsocket.socket, &buflen);
		if (buf == NULL)
		{
			clean(proxysocket.socket, redirectsocket.socket, clientsocket, buf);
			return 1;
		}
		printf("redirect msg: \n%s\n", buf);

		send(clientsocket, buf, buflen, 0);
		if (sendres == -1)
		{
			perror("Couldnt send to client with clientsocket");
			clean(proxysocket.socket, redirectsocket.socket, clientsocket, buf);
			return 1;
		}

		// Don't close proxysocket since it might be needed on the next iteration
		clean(-1, redirectsocket.socket, clientsocket, buf);
	}
	
	shutdown(proxysocket.socket, SHUT_RDWR);

	return 0;
}

