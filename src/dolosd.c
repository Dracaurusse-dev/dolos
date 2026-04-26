#include "stringutils.h"

#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>


#define BUFFER_SIZE  255


int main(void)
{
	char *ip_redirect, ip_server;
	char *chance_type;
	uint16_t chance_value, portredirect;

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

		
		if (strncmp(buffer, "bind", strlen("bind")) == 0)
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
		}

		buffer = trimspaces(buffer);
		if (buffer == NULL)
		{
			fprintf(stderr, "buffer is null\n");
			free(buffer);
			buffer = (char *) malloc(BUFFER_SIZE * sizeof(char));
			continue;
		}

		if (strncmp(buffer, "ip_redirect", strlen("ip_redirect") * sizeof(char)) == 0)
		{
			ip_redirect = cutstr(buffer, '=', '\n', XCLUDE_END | XCLUDE_START);

			printf("ipredirect: %s\n", ip_redirect);
		}

		if (strncmp(buffer, "chance_type", strlen("chance_type") * sizeof(char)) == 0)
		{
			chance_type = cutstr(buffer, '=', '\n', XCLUDE_START | XCLUDE_END);

			printf("Chance type: %s\n", chance_type);
		}

		if (strncmp(buffer, "chance_value", strlen("chance_value") * sizeof(char)) == 0)
		{
			char *chance_value_str = cutstr(buffer, '=', '\n', XCLUDE_START | XCLUDE_END);
			chance_value = strtou16(chance_value_str);

			printf("Chance value: %d\n", chance_value);
		}

		if (strncmp(buffer, "port_redirect", strlen("port_redirect") * sizeof(char)) == 0)
		{
			char *portredirectstr = cutstr(buffer, '=', '\n', XCLUDE_START | XCLUDE_END);
			portredirect = strtou16(portredirectstr);

			printf("Port redirect: %d\n", portredirect);
		}

		free(buffer);
		buffer = (char *) malloc(BUFFER_SIZE * sizeof(char));
	}

	free(buffer);
	
	return 0;
}
