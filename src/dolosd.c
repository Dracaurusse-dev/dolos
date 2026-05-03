#include "stringutils.h"

#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>


#define BUFFER_SIZE  255
#define FLAG_COUNT 11


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


typedef struct
{
	char *flag;
	char *valuestr;
	uint32_t valueint;
	uint8_t useint;
} Argument;


uint8_t isvar(char *buffer, char *var)
{
	return strncmp(buffer, var, strlen(var) * sizeof(char)) == 0;
}


void freesettings(ConfigSettings settings)
{
	free(settings.ip_redirect);
	free(settings.ip_server);
	free(settings.target);
	free(settings.chance_type);
}


void genargs(char **argv, ConfigSettings settings, uint16_t proxyport, uint16_t serverport)
{
	Argument args[FLAG_COUNT] = 
	{
		{"/usr/bin/dolos-proxy", NULL, 0, 0},
		{"-I", settings.ip_server, 0, 0},
		{"-i", settings.ip_redirect, 0, 0},
		{"-w", NULL, serverport, 1},
		{"-r", NULL, settings.port_redirect, 1},
		{"-p", NULL, proxyport, 1},
		{"-c", NULL, settings.max_socket_connection, 1},
		{"-t", settings.target, 0, 0},
		{"-T", settings.chance_type, 0, 0},
		{"-V", NULL, settings.chance_value, 1},
		{"-j", NULL, settings.job_amnt, 1}
	};

	size_t j = 0;
	for (size_t i = 0; i < FLAG_COUNT; i++)
	{
		if (args[i].useint)
		{
			args[i].valuestr = (char *) calloc(8, sizeof(char));
			sprintf(args[i].valuestr, "%d", args[i].valueint);
		}
		argv[j++] = args[i].flag;
		if (args[i].valuestr == NULL)
			continue;

		argv[j++] = args[i].valuestr;
	}
	argv[j++] = 0;
}


void checkforvar(char *buffer, ConfigSettings *settings)
{
	if (isvar(buffer, "ip_redirect"))
	{
		settings->ip_redirect = cutstr(buffer, '=', '\n', XCLUDE_END | XCLUDE_START);

		printf("ipredirect: %s\n", settings->ip_redirect);
	}
	else if (isvar(buffer, "chance_type"))
	{
		settings->chance_type = cutstr(buffer, '=', '\n', XCLUDE_START | XCLUDE_END);

		printf("Chance type: %s\n", settings->chance_type);
	}
	else if (isvar(buffer, "chance_value"))
	{
		char *chance_value_str = cutstr(buffer, '=', '\n', XCLUDE_START | XCLUDE_END);
		settings->chance_value = strtou16(chance_value_str);
		free(chance_value_str);

		printf("Chance value: %d\n", settings->chance_value);
	}
	else if (isvar(buffer, "port_redirect"))
	{
		char *portredirectstr = cutstr(buffer, '=', '\n', XCLUDE_START | XCLUDE_END);
		settings->port_redirect = strtou16(portredirectstr);
		free(portredirectstr);

		printf("Port redirect: %d\n", settings->port_redirect);
	}
	else if (isvar(buffer, "ip_server"))
	{
		settings->ip_server = cutstr(buffer, '=', '\n', XCLUDE_START | XCLUDE_END);

		printf("ipserver: %s\n", settings->ip_server);
	}
	else if  (isvar(buffer, "target"))
	{
		settings->target = cutstr(buffer, '=', '\n', XCLUDE_START | XCLUDE_END);

		printf("target: %s\n", settings->target);
	}
	else if (isvar(buffer, "max_socket_connection"))
	{
		char *sockconnstr = cutstr(buffer, '=', '\n', XCLUDE_START | XCLUDE_END);

		settings->max_socket_connection = strtou8(sockconnstr);
		free(sockconnstr);

		printf("max socket connection: %d\n", settings->max_socket_connection);
	}
	else if (isvar(buffer, "job_amnt"))
	{
		char *jobstr = cutstr(buffer, '=', '\n', XCLUDE_START | XCLUDE_END);

		settings->job_amnt = strtou8(jobstr);
		free(jobstr);

		printf("Job number: %d\n", settings->job_amnt);
	}
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

	char *buffer = (char *) calloc(BUFFER_SIZE, sizeof(char));
	while (fgets(buffer, BUFFER_SIZE, fp) != NULL)
	{
		if (strlen(buffer) <= 1)
		{
			// Can memset instead of reallocating since no memory operation really happened
			memset(buffer, 0, BUFFER_SIZE * sizeof(char));
			continue;
		}

		char *tmpbuf = strdup(buffer);
		free(buffer);
		buffer = removecomments(tmpbuf, '#');
		free(tmpbuf);
		if (buffer == NULL)
		{
			free(buffer);
			buffer = (char *) calloc(BUFFER_SIZE, sizeof(char));
			continue;
		}

		
		if (isvar(buffer, "bind"))
		{
			char *ports = cutstr(buffer, ' ', '\n', XCLUDE_END | XCLUDE_START);

			char *proxyportstr  = cutstr(ports, 0, ' ', XCLUDE_END);
			char *serverportstr = cutstr(ports, ' ', '\n', XCLUDE_END | XCLUDE_START);
			uint16_t proxyport  = strtou16(proxyportstr);
			uint16_t serverport = strtou16(serverportstr);

			free(proxyportstr);
			free(serverportstr);
			free(ports);

			printf("Proxyport is %d and serverport is %d\n", proxyport, serverport);

			// TODO: later, use threads
			pid_t pid = fork();
			if (pid < 0)
			{
				perror("Fork failed for dolosd");

				free(buffer);
				freesettings(settings);
				fclose(fp);
				//buffer = (char *) calloc(BUFFER_SIZE, sizeof(char));
				return 1;
			}
			
			if (pid == 0)
			{
				char *argv[FLAG_COUNT * 2];
				genargs(argv, settings, proxyport, serverport);

				execv(argv[0], argv);

				free(buffer);
				freesettings(settings);
				fclose(fp);
				//buffer = (char *) calloc(BUFFER_SIZE, sizeof(char));
				return 0;

			}

			free(buffer);
			buffer = (char *) calloc(BUFFER_SIZE, sizeof(char));
			continue;
		}

		tmpbuf = strdup(buffer);
		free(buffer);
		buffer = trimspaces(tmpbuf);
		free(tmpbuf);
		if (buffer == NULL)
		{
			fprintf(stderr, "buffer is null after triming\n");
			free(buffer);
			buffer = (char *) calloc(BUFFER_SIZE, sizeof(char));
			continue;
		}

		checkforvar(buffer, &settings);

		free(buffer);
		buffer = (char *) calloc(BUFFER_SIZE, sizeof(char));
	}

	freesettings(settings);
	free(buffer);
	fclose(fp);
	
	return 0;
}
