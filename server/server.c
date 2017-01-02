#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <sys/wait.h>
#include <signal.h>

#define BACKLOG 10     // how many pending connections queue will hold
#define MAX_PORT_LENGTH 10

int bind_server_to_port(struct addrinfo *servinfo);

int main(int argc, char *argv[]) {
	char server_port[MAX_PORT_LENGTH];
	struct addrinfo *servinfo, aux;
	int ret, socket_fd;

	if (argc != 2) { 
		fprintf(stderr, "Usage: server_port");
		exit(1);
	}
	strcpy(server_port, argv[1]);

	memset(&aux, 0, sizeof(aux));
	aux.ai_family = AF_UNSPEC; // both IPv4 and IPv6
	aux.ai_socktype  = SOCK_STREAM; // TCP
	aux.ai_flags = AI_PASSIVE; // use IP of the host
	
	if ((ret = getaddrinfo(NULL, server_port, &aux, &servinfo)) != 0) {
                fprintf(stderr, "getaddrinfo failed: %s\n", gai_strerror(ret));
                exit(1);
	}
	
	// get socket and bind it to the given port
	socket_fd = bind_server_to_port(servinfo);
	freeaddrinfo(servinfo);

	// listen

	// serve incoming clients
	return 0;
}

int bind_server_to_port(struct addrinfo *servinfo) {
	struct addrinfo *p;
	int socket_fd;
	for(p = servinfo; p != NULL; p = p->ai_next) {
		// try to create a socket
		if ((socket_fd = socket(p->ai_family, p->ai_socktype, p->ai_protocol)) == -1) {
			perror("Failed to create a socket");
			continue;
		}	

		// recover free ports

		// bind socket to its port
		if (bind(socket_fd, p->ai_addr, p->ai_addrlen) == -1) {
			close(socket_fd);
			perror("Failed to bind socket to port");
			continue;
		}
	}
}

