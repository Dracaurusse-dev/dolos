#ifndef STRINGSUTILS_H
#define STRINGSUTILS_H


#include <stdint.h>
#include <stddef.h>


#define XCLUDE_START	1
#define XCLUDE_END 	2
#define CUTSTR_VERBOSE	4


char *cutstr(char *buf, char startchar, char endchar, uint8_t opts);
char *trimspaces(char *s);
char *removecomments(char *s, const char c);

uint8_t strtou8(char *);
uint16_t strtou16(char *s);


#endif
