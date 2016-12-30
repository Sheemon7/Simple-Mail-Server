#include <sys/socket.h>
#include <stdlib.h>
#include <stdio.h>

#ifndef CREATE_SOCKET_H
#define CREATE_SOCKET_H

int create_socket(int addr_type, int stream, int protocol) {
	int socketfd = socket(addr_type, stream, protocol);
	if (socketfd < 0) {
		fprintf(stderr, "Couldn't create a socket!");
		exit(1);
	}
	return socketfd;
}

#endif
