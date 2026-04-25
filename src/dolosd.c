#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <ctype.h>


#define BUFFER_SIZE  255

#define XCLUDE_START    1
#define XCLUDE_END   	2
#define CUTSTR_VERBOSE	4


char *firstnchars(const char *src, size_t srcsize, size_t dstsize)
{
	if (dstsize == 0)
		return NULL;

	char *dst = (char *)malloc(dstsize);
	char *tmp = (char *)malloc(srcsize * sizeof(*src));

	strcpy(tmp, src);
	tmp[dstsize] = 0;
	strcpy(dst, tmp);

	return dst;
}


char *cutstr(char *buf, char startchar, char endchar, uint8_t opts)
{
	/*
	 * OPTS:
	 *	XCLUDE_START: should exclude startchar from the buffer
	 * 	XCLUDE_END: should exclude endchar from the buffer
	 * 	CUTSTR_VERBOSE: should print more logs
	 */
	// TODO: do errno to understand what happened since the return value is always NULL on error
	if (buf == NULL)
	{	
		perror("buf is null");
		return NULL;
	}

	char *resbuf;
	uint32_t excludevalues = (opts & XCLUDE_START) + (opts & XCLUDE_END);
	ssize_t startid = -1;
	if (startchar == 0)
		startid = XCLUDE_START & opts;
	
	if (opts & CUTSTR_VERBOSE)
	{
		printf("startchar %c startid %ld \n", startchar, startid);

		printf("startid before loop: %ld\n", startid);
	}

	size_t starti = 0;
	if (startid != -1)
	{
		resbuf = (char *) malloc(strlen(buf) - excludevalues);
		starti = startid;
	}
	
	for (size_t i = starti; i < strlen(buf); i++)
	{
		if (buf[i] == startchar && startid == -1)
		{
			startid = i;

			if (opts & CUTSTR_VERBOSE)
				printf("excludevalues: %d\n", excludevalues);

			resbuf = (char *)malloc(strlen(buf) - i - excludevalues);
			if (opts & XCLUDE_START)
				startid++;

			continue;
		}

		if (startid == -1)
			continue;

		if (buf[i] == endchar)
		{
			if (XCLUDE_END)
				break;
		}

		resbuf[i - startid] = buf[i];

		if (opts & CUTSTR_VERBOSE)
		{
			printf("i - startid: %ld\n", i - startid);
			printf("buf[i] = resbuf[i - startid] = %c\n", buf[i]);
			printf("Current resbuf: %s\n", resbuf);
		}

		
	}

	if (opts & CUTSTR_VERBOSE)
		printf("resbuf before return: %s\n", resbuf);
	
	return resbuf;
}

char *trimspaces(char *s)
{
	if (s == NULL)
	{
		fprintf(stderr, "input string is null\n");
		return NULL;
	}

	if (s[0] == 0 || s[0] == '\n')
	{
		fprintf(stderr, "input string is empty\n");
		return NULL;
	}

	size_t spaceamnt = 0;

	for (char *c = s; *c; c++)
	{
		if (isspace(*c))
			spaceamnt++;
	}

	size_t resstrsize = strlen(s) - spaceamnt + 1;
	char *resstr = (char *)malloc(resstrsize);
	
	size_t inputid = 0;
	size_t outputid = 0;
	while (inputid < strlen(s) && outputid < resstrsize)
	{
		if (!isspace(s[inputid]))
			resstr[outputid++] = s[inputid];
		
		inputid++;
	}

	return resstr;
}


char *removecomments(char *s, const char c)
{
	if (s[0] == c)
		return NULL;
	
	ssize_t realsize = 0;
	char *res = (char *) malloc(sizeof(char));

	if (res == NULL)
	{
		fprintf(stderr, "could'nt allocate memory when removing comments");
		return NULL;
	}

	for (char *x = s; *x; x++)
	{
		realsize++;

		res = realloc(res, realsize);
		if (res == NULL)
		{
			fprintf(stderr, "Couldnt reallocate memory when removing comments");
			free(res);
			return NULL;
		}

		if (*x == c)
		{
			res[realsize - 1] = 0;
			break;
		}
		
		res[realsize - 1] = *x;

	}

	return res;
}


uint16_t strtou16(char *s)
{
	char *c;
	for (c = s; *c; c++)
	{
		if (!isdigit(*c))
			return 0L;
	}

	return (uint16_t) strtoul(s, 0L, 10);
}


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
