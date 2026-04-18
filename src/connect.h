#ifndef CONNECT_H
#define CONNECT_H


#include <netinet/in.h>
#include <stdint.h>


typedef struct {
	struct sockaddr_in addr;
	int32_t socket, opt;
} Socket;


int32_t opensocket(Socket *socket, uint16_t port);
int32_t connecttoapache(uint32_t port, Socket *redirect);
int32_t connectclient();
void clean(int32_t proxysocket, int32_t redirectsocket, int32_t clientsocket, char *buf);

#endif  // CONNECT_H
