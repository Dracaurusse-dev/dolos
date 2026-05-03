#include "connect.h"
#include "stringutils.h"

#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <stdlib.h>
#include <time.h>

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
	uint16_t  active_port;
	uint8_t   max_socket_conn;
	uint8_t   chance_value;
	uint8_t   count_value;
	char 	  *chance_type;
	char 	  *server_ip;
	char 	  *redirect_ip;
	char 	  *target;
} Settings;


char *longrecv(int32_t socket, ssize_t *lengthoutput)
{
	ssize_t length = BUFFER_SIZE;
	ssize_t res;
	char *buf = (char *)calloc(length, sizeof(char));
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
		buf = (char *) realloc(buf, length * sizeof(char));
		memset(buf, 0, length * sizeof(char));

		if (buf == NULL)
		{
			perror("Couldn't realloc buf");
			return NULL;
		}
	}
	while (res >= length-incrementamnt);

	//buf = (char *) realloc(buf, length * sizeof(char));
	//memset(buf, 0, length * sizeof(char));
	free(buf);
	buf = (char *) calloc(length, sizeof(char));
	*lengthoutput = recv(socket, buf, length, 0);

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
			settings->active_port = settings->website_port;
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
			fprintf(stderr, "Unknown argument passed to setvalue: %c\n", arg);
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


uint8_t is_get_html_req(char *req)
{
	if (!strncmp(req, "GET /", strlen("GET /")))
		return true;		
	if (!strncmp(req, "GET /index", strlen("GET /index")))
		return true;

	return false;
}


uint8_t handlerandom(Settings *settings)
{
	if (settings == NULL)
	{
		fprintf(stderr, "Pointer to NULL");
		return 1;
	}

	if (strcmp(settings->chance_type, "%") == 0)
	{
		if ( rand() % 100 < settings->chance_value - 1)
			settings->active_port = settings->redirect_port;
		else
			settings->active_port = settings->website_port;
	}
	else if (strcmp(settings->chance_type, "COUNT") == 0)
	{
		// If max count and curr count are equal, then user already has been redirected
		// and the redirect should reset
		if (settings->count_value == settings->chance_value) 
		{
			settings->count_value = 0;
			settings->active_port = settings->website_port;
		}

		settings->count_value++;

		// If max count and curr count are equal, then user should be redirected
		if (settings->count_value == settings->chance_value)
			settings->active_port = settings->redirect_port;
	}

	return 0;
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
		.active_port 	 = 	WEBSITE_DEFAULT_PORT,
		.max_socket_conn =	DEFAULT_MAX_SOCKET_CONN,
		.chance_value 	 = 	DEFAULT_CHANCE_VALUE,
		.count_value 	 = 	0,
		.chance_type  	 = 	DEFAULT_CHANCE_TYPE,
		.server_ip 	 = 	DEFAULT_SERVER_IP,
		.redirect_ip 	 =	DEFAULT_REDIRECT_IP,
		.target 	 =	DEFAULT_TARGET
	};

	uint8_t argres = parseargs(argc, argv, &settings);
	if (argres != PARSED_SUCCESSFULLY)
	{
		fprintf(stderr, "Failed args parsing with error %d\n", argres);
		return 1;
	}

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
			continue;
		}

		// Read the request from the client	
		ssize_t reqbuflen;
		char *reqbuf = longrecv(clientsocket, &reqbuflen);
		if (reqbuf == NULL)
		{
			shutdown(proxysocket.socket, SHUT_RDWR);
			shutdown(redirectsocket.socket, SHUT_RDWR);
			shutdown(clientsocket, SHUT_RDWR);
			free(reqbuf);
			return 1;
		}

		//printf("client message: \n%s\n", reqbuf);
		
		if (is_get_html_req(reqbuf) == 0)
		{
			uint8_t rdres = handlerandom(&settings);
			if (rdres != 0)
			{
				printf("cant handle random");
				shutdown(proxysocket.socket, SHUT_RDWR);
				shutdown(redirectsocket.socket, SHUT_RDWR);
				shutdown(clientsocket, SHUT_RDWR);
				return 1;
			}
		}

		int32_t res = connecttoapache(settings.active_port, &redirectsocket);
		if (res != 0)
		{
			perror("Couldnt connect to apache");
			shutdown(proxysocket.socket, SHUT_RDWR);
			shutdown(redirectsocket.socket, SHUT_RDWR);
			shutdown(clientsocket, SHUT_RDWR);
			free(reqbuf);
			return 1;
		}

		// Send the packet to the server
		int32_t sendres = send(redirectsocket.socket, reqbuf, reqbuflen, 0);
		if (sendres == -1)
		{
			perror("Couldnt send to server with redirectsocket");
			shutdown(proxysocket.socket, SHUT_RDWR);
			shutdown(redirectsocket.socket, SHUT_RDWR);
			shutdown(clientsocket, SHUT_RDWR);
			free(reqbuf);
			return 1;
		}

		// Receive the reply and send it to the client
		free(reqbuf);

		ssize_t repbuflen;
		char *repbuf = longrecv(redirectsocket.socket, &repbuflen);
		if (repbuf == NULL)
		{
			shutdown(proxysocket.socket, SHUT_RDWR);
			shutdown(redirectsocket.socket, SHUT_RDWR);
			shutdown(clientsocket, SHUT_RDWR);
			free(repbuf);
			return 1;
		}
		//printf("redirect msg: \n%s\n", repbuf);

		send(clientsocket, repbuf, repbuflen, 0);
		if (sendres == -1)
		{
			perror("Couldnt send to client with clientsocket");
			shutdown(proxysocket.socket, SHUT_RDWR);
			shutdown(redirectsocket.socket, SHUT_RDWR);
			shutdown(clientsocket, SHUT_RDWR);
			free(repbuf);
			return 1;
		}

		// Don't close proxysocket since it might be needed on the next iteration
		shutdown(redirectsocket.socket, SHUT_RDWR);
		shutdown(clientsocket, SHUT_RDWR);
		free(repbuf);
	}
	
	shutdown(proxysocket.socket, SHUT_RDWR);

	return 0;
}

