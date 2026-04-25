#include "stringutils.h"

#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>


#define BUFFER_SIZE  255


int main(void)
{
	char *ip_redirect;
	//uint8_t chance_type;
	//uint16_t chance_value;

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

		
		if (strcmp(firstnchars(buffer, strlen(buffer), 4), "bind") == 0)
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

		if (strcmp(firstnchars(buffer, strlen(buffer), strlen("ip_redirect")), "ip_redirect") == 0)
		{
			ip_redirect = cutstr(buffer, '=', '\n', XCLUDE_END | XCLUDE_START);

			printf("ipredirect: %s\n", ip_redirect);
		}

		free(buffer);
		buffer = (char *) malloc(BUFFER_SIZE * sizeof(char));
	}

	free(buffer);
	
	
	return 0;
}
