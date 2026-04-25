#include "connect.h"
#include "stringutils.h"

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


#define BUFFER_SIZE 		4096

#define DEFAULT_PORT 		8080
#define WEBSITE_DEFAULT_PORT 	80
#define REDIRECT_DEFAULT_PORT	8000
#define DEFAULT_SERVER_IP	"127.0.0.1"
#define DEFAULT_REDIRECT_IP	"127.0.0.1"
#define DEFAULT_TARGET		"*"
#define DEFAULT_CHANCE_TYPE	"COUNT"
#define DEFAULT_CHANCE_VALUE	100
#define DEFAULT_MAX_SOCKET_CONN 5

#define PARSED_SUCCESSFULLY 	1
#define NO_ARGS_PASSED 		2
#define INVALID_ARG		4


typedef struct 
{
	uint16_t  proxy_port;
	uint16_t  website_port;
	uint16_t  redirect_port;
	uint8_t   max_socket_conn;
	uint8_t   chance_value;
	char 	  *chance_type;
	char 	  *server_ip;
	char 	  *redirect_ip;
	char 	  *target;
} Settings;


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


uint8_t setvalue(char arg, char *value, Settings *settings)
{
	switch (arg)
	{
		case 'p':
			settings->proxy_port = strtou16(value);
			break;
		case 'w':
			settings->website_port = strtou16(value);
			break;
		case 'r':
			settings->redirect_port = strtou16(value);
			break;
		
		case 'I':
			settings->server_ip = value;
			break;
		case 'i':
			settings->redirect_ip = value;
			break;

		case 'c':
			settings->max_socket_conn = strtou8(value);
			break;
		
		case 't':
			settings->target = value;
			break;

		case 'T':
			settings->chance_type = value;
			break;
		case 'V':
			settings->chance_value = strtou8(value);
			break;

		default:
			fprintf(stderr, "Unknown argument passed to setvalue");
			return 1;
	}

	return 0;
}


uint8_t parseargs(int argc, char **argv, Settings *settings)
{
	if (argc == 1)
	{
		puts("no argument passed, using default values");
		return NO_ARGS_PASSED;
	}

	uint8_t isvalue = 0;

	for (int i = 1; i < argc; i++)
	{
		if (isvalue)
		{
			isvalue = 0;
			continue;
		}
		
		char arg;

		if (argv[i][0] == '-')
		{
			isvalue = 1;

			if (argv[i][1] == 0 || argv[i][0] == '\n')
			{
				fprintf(stderr, "Invalid arguement after '-': %s", argv[i]);
				return INVALID_ARG;
			}

			arg = argv[i][1];
		}

		if (argv[i][0] != '-')
		{
			arg = argv[i][0];
		}

		if (setvalue(arg, argv[i + 1], settings) == 1)
		{
			return INVALID_ARG;
		}

	}

	return PARSED_SUCCESSFULLY;
}


int main(int argc, char **argv)
{
	int32_t clientsocket;
	Socket proxysocket, redirectsocket;
	Settings settings = 
	{
		.proxy_port 	 =	DEFAULT_PORT,
		.website_port 	 = 	WEBSITE_DEFAULT_PORT,
		.redirect_port 	 = 	REDIRECT_DEFAULT_PORT,
		.max_socket_conn =	DEFAULT_MAX_SOCKET_CONN,
		.chance_value 	 = 	DEFAULT_CHANCE_VALUE,
		.chance_type  	 = 	DEFAULT_CHANCE_TYPE,
		.server_ip 	 = 	DEFAULT_SERVER_IP,
		.redirect_ip 	 =	DEFAULT_REDIRECT_IP,
		.target 	 =	DEFAULT_TARGET
	};

	parseargs(argc, argv, &settings);

	int32_t openres = opensocket(&proxysocket, settings.proxy_port, settings.max_socket_conn);
	if (openres == 1)
	{
		perror("Couldn't open the socket");
		return 1;
	}

	while (1)
	{
		clientsocket = connectclient(proxysocket.socket);
		if (clientsocket == -1)
		{
			break;
		}

		int32_t res = connecttoapache(settings.website_port, &redirectsocket);
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

