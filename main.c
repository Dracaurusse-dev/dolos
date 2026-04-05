#include <stdio.h>
#include <stdint.h>
#include <string.h>

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


int main(void)
{
	Socket serversocket;
	if (opensocket(&serversocket) == 1)
		return 1;	

	listen(serversocket.socket, 5);
	int clientsocket = accept(serversocket.socket, nullptr, nullptr);
	
	// Read the first request
	char buffer[1024] = {0};
	recv(clientsocket, buffer, sizeof(buffer), 0);
	printf("Message %s\n", buffer);

	// Send a custom msg to the client
	/* 
	char *response = "<!DOCTYPE HTML PUBLIC \"-//W3C//DTD HTML 4.01//EN\" \"http://www.w3.org/TR/html4/strict.dtd\">\n<html>\n<head>\n<title>It works! Apache httpd</title>\n</head>\n<body>\n<p>It works!</p>\n</body>\n</html>\n";

	send(clientsocket, response, strlen(response), 0);
	*/

	struct sockaddr_in apaddr, cli;

	int32_t redirectsocket = socket(AF_INET, SOCK_STREAM, 0);
	if (redirectsocket  == -1)
	{
		perror("Couldnt create redirect socket");
		return 1;
	}

	bzero(&apaddr, sizeof(apaddr));
	apaddr.sin_family = AF_INET;
	apaddr.sin_addr.s_addr = inet_addr("127.0.0.1");
	apaddr.sin_port = htons(80);

	if (connect(redirectsocket, (struct sockaddr *)&apaddr, sizeof(apaddr)) != 0)
	{
		perror("Connection failed");
		return 1;
	}

	send(redirectsocket, buffer, strlen(buffer), 0);
	recv(redirectsocket, buffer, sizeof(buffer), 0);
	send(clientsocket, buffer, strlen(buffer), 0);

	close(clientsocket);
	close(redirectsocket);
	close(serversocket.socket);

	return 0;
}

