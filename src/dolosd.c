#include "stringutils.h"

#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>


#define BUFFER_SIZE  255


typedef struct
{
	char *ip_redirect;
	char *ip_server;
	char *target;
	char *chance_type;
	uint16_t chance_value;
	uint16_t port_redirect;
	uint8_t max_socket_connection;
	uint8_t job_amnt;
} ConfigSettings;


uint8_t isvar(char *buffer, char *var)
{
	return strncmp(buffer, var, strlen(var) * sizeof(char)) == 0;
}


int main(void)
{
	ConfigSettings settings;

	FILE *fp = fopen("/etc/dolos.conf", "r");
	if (fp == NULL)
	{
		perror("Couldn't open /etc/dolos.conf");
		return 1;
	}

	char *buffer = malloc(BUFFER_SIZE * sizeof(char));
	while (fgets(buffer, BUFFER_SIZE, fp) != NULL)
	{
		buffer = removecomments(buffer, '#');
		if (buffer == NULL)
		{
			free(buffer);
			buffer = (char *) malloc(BUFFER_SIZE * sizeof(char));
			continue;
		}

		
		if (isvar(buffer, "bind"))
		{
			char *ports = cutstr(buffer, ' ', '\n', XCLUDE_END | XCLUDE_START);

			char *proxyportstr  = cutstr(ports, 0, ' ', XCLUDE_END);
			char *serverportstr = cutstr(ports, ' ', '\n', XCLUDE_END | XCLUDE_START);
			uint16_t proxyport  = strtou16(proxyportstr);
			uint16_t serverport = strtou16(serverportstr);

			printf("Proxyport is %d and serverport is %d\n", proxyport, serverport);

			free(buffer);
			buffer = (char *) malloc(BUFFER_SIZE * sizeof(char));
			continue;
			//TODO: fork process for the  bind
		}

		buffer = trimspaces(buffer);
		if (buffer == NULL)
		{
			fprintf(stderr, "buffer is null after triming\n");
			free(buffer);
			buffer = (char *) malloc(BUFFER_SIZE * sizeof(char));
			continue;
		}

		// TODO: maybe do another settings struct and pass it to a function to hide this if shit
		if (isvar(buffer, "ip_redirect"))
		{
			settings.ip_redirect = cutstr(buffer, '=', '\n', XCLUDE_END | XCLUDE_START);

			printf("ipredirect: %s\n", settings.ip_redirect);
		}
		else if (isvar(buffer, "chance_type"))
		{
			settings.chance_type = cutstr(buffer, '=', '\n', XCLUDE_START | XCLUDE_END);

			printf("Chance type: %s\n", settings.chance_type);
		}
		else if (isvar(buffer, "chance_value"))
		{
			char *chance_value_str = cutstr(buffer, '=', '\n', XCLUDE_START | XCLUDE_END);
			settings.chance_value = strtou16(chance_value_str);

			printf("Chance value: %d\n", settings.chance_value);
		}
		else if (isvar(buffer, "port_redirect"))
		{
			char *portredirectstr = cutstr(buffer, '=', '\n', XCLUDE_START | XCLUDE_END);
			settings.port_redirect = strtou16(portredirectstr);

			printf("Port redirect: %d\n", settings.port_redirect);
		}
		else if (isvar(buffer, "ip_server"))
		{
			settings.ip_server = cutstr(buffer, '=', '\n', XCLUDE_START | XCLUDE_END);

			printf("ipserver: %s\n", settings.ip_server);
		}
		else if  (isvar(buffer, "target"))
		{
			settings.target = cutstr(buffer, '=', '\n', XCLUDE_START | XCLUDE_END);

			printf("target: %s\n", settings.target);
		}
		else if (isvar(buffer, "max_socket_connection"))
		{
			char *sockconnstr = cutstr(buffer, '=', '\n', XCLUDE_START | XCLUDE_END);

			settings.max_socket_connection = strtou8(sockconnstr);

			printf("max socket connection: %d\n", settings.max_socket_connection);
		}
		else if (isvar(buffer, "job_amnt"))
		{
			char *jobstr = cutstr(buffer, '=', '\n', XCLUDE_START | XCLUDE_END);

			settings.job_amnt = strtou8(jobstr);

			printf("Job number: %d\n", settings.job_amnt);
		}

		free(buffer);
		buffer = (char *) malloc(BUFFER_SIZE * sizeof(char));
	}

	free(buffer);
	
	return 0;
}
