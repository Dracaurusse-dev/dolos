#ifndef CONNECT_H
#define CONNECT_H


#include <netinet/in.h>
#include <stdint.h>


typedef struct {
	struct sockaddr_in addr;
	int32_t socket, opt;
} Socket;


int32_t opensocket(Socket *socket, uint16_t port, uint8_t connection_amnt);
int32_t connecttoapache(uint32_t port, Socket *redirect);
int32_t connectclient(int32_t proxysocket);

#endif  // CONNECT_H
