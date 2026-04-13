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

// TODO: macro pour fermer les socket et free le buffer
// TODO: le buffer il faut le malloc au début du while (dedans) et realloc régulièrement, en gros je pense while recv(socket, buffer, strlen(buffer) != 0 do smth je sais pas trop
// TODO: quand on créer un socket (ou alloue la mémoire pour le buffer) on define une variable et dans la macro on fait genre ifdef VARIABLE_SET free ou close et undefine VARIABLE_SET (jsp si c'est undefine) 
typedef struct {
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


void closesockets()
{
	#ifdef PROXY_SOCKET
		close(PROXY_SOCKET);
		#undef PROXY_SOCKET
	#endif

	#ifdef CLIENT_SOCKET
		close(CLIENT_SOCKET);
		#undef CLIENT_SOCKET
	#endif

	#ifdef REDIRECT_SOCKET
		close(REDIRECT_SOCKET);
		#undef REDIRECT_SOCKET
	#endif

	#ifdef RECV_BUFFER 
		printf("Free buffer");
		free(RECV_BUFFER);
		#undef RECV_BUFFER
	#endif
	/*
	for (int32_t i = 0; i < socketcount; i++)
	{
		int32_t r = close(sockets[i]);
		if (r != 0)
			fprintf(stderr, "socket %d couldnt close", i);
	}*/

	//TODO: check later if it correctly works, it should but im unsure
}


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

	int32_t openres = opensocket(&proxysocket);
	#define PROXY_SOCKET proxysocket.socket
	if (openres == 1)
	{
		closesockets();
		return 1;
	}

	if (listen(proxysocket.socket, MAX_SOCKET_CONN) < 0)
	{
		closesockets();
		return 1;
	}

	while (1)
	{
		clientsocket = accept(proxysocket.socket, NULL, NULL);
		if (clientsocket == -1)
		{
			perror("Error connecting to client");
			close(clientsocket);
			close(proxysocket.socket);
			return 1;
		}

		// Establish a connection to apache
		int32_t res = connecttoapache(WEBSITE_PORT, &redirectsocket);
		if (res != 0)
		{
			perror("Couldnt connect to apache");
			close(clientsocket);
			close(proxysocket.socket);
			close(redirectsocket.socket);
			return 1;
		}

		// Read the request from the client	
		ssize_t buflen;
		char *buf = longrecv(clientsocket, &buflen);
		if (buf == NULL)
		{
			close(clientsocket);
			close(redirectsocket.socket);
			close(proxysocket.socket);
			free(buf);
			return 1;
		}
		printf("client message: \n%s\n", buf);

		// Send the packet to the server
		int32_t sendres = send(redirectsocket.socket, buf, buflen, 0);
		if (sendres == -1)
		{
			perror("Couldnt send to server with redirectsocket");
			close(clientsocket);
			close(redirectsocket.socket);
			close(proxysocket.socket);
			free(buf);
			return 1;
		}

		// Receive the reply and send it to the client
		buf = longrecv(redirectsocket.socket, &buflen);
		if (buf == NULL)
		{
			close(clientsocket);
			close(redirectsocket.socket);
			close(proxysocket.socket);
			free(buf);
			return 1;
		}
		printf("redirect msg: \n%s\n", buf);

		send(clientsocket, buf, buflen, 0);
		if (sendres == -1)
		{
			perror("Couldnt send to client with clientsocket");
			close(clientsocket);
			close(redirectsocket.socket);
			close(proxysocket.socket);
			free(buf);
			return 1;
		}

		close(clientsocket);
		close(redirectsocket.socket);
		free(buf);
	}
	
	//closesockets();
	close(clientsocket);
	close(redirectsocket.socket);
	close(proxysocket.socket);

	return 0;
}

