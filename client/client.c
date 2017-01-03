#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <netdb.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include "clientModule.h"

#define MAX_LOGIN_LENGTH 20
#define MAX_PORT_LENGTH 10
#define MAX_IP_LENGTH 20

void set_args(char *argv[], char *server_ip, char *server_port, char *login);
int connect_to_server(struct addrinfo *servinfo);

int main(int argc, char *argv[]) {
	char server_ip[MAX_IP_LENGTH];
	char server_port[MAX_PORT_LENGTH];	
	char login[MAX_LOGIN_LENGTH];	
	struct addrinfo aux, *servinfo, *p;
	int ret, socket_fd;

	if (argc != 4) {
		fprintf(stderr, "Usage: server_IP, server_port, login");
		exit(1);
	}
	set_args(argv, server_ip, server_port, login);

	memset(&aux, 0, sizeof(aux));
	aux.ai_family = AF_UNSPEC; // both IPv4 and IPv6
	aux.ai_socktype = SOCK_STREAM; // TCP
	
	if ((ret = getaddrinfo(server_ip, server_port, &aux, &servinfo)) != 0) {
		fprintf(stderr, "getaddrinfo failed: %s\n", gai_strerror(ret));
		exit(1);
	}	
	
	printf("Getting to the first conforming result with address %s on port %s\n", server_ip, server_port);
	socket_fd = connect_to_server(servinfo);
	freeaddrinfo(servinfo);
	
	// communication
	// move to another module afterwards!
	int MAXDATASIZE = 100;
	char buf[MAXDATASIZE];
	int numbytes;
	if ((numbytes = recv(socket_fd, buf, MAXDATASIZE-1, 0)) == -1) {
        perror("recv");
        exit(1);
    }

    buf[numbytes] = '\0';	
	printf("client: received '%s'\n",buf);
	clientModule(socket_fd, login);
	close(socket_fd);
	return 0;
}

void set_args(char *argv[], char *server_ip, char *server_port, char *login) {
	strcpy(server_ip, argv[1]);
	strcpy(server_port, argv[2]);
	strcpy(login, argv[3]);
}

int connect_to_server(struct addrinfo *servinfo) {
	struct addrinfo *p;
	int socket_fd;
	for(p = servinfo; p != NULL; p = p->ai_next) {
		// try to create a socket
		if ((socket_fd = socket(p->ai_family, p->ai_socktype, p->ai_protocol)) == -1) {
			perror("Failed to create a socket");
			continue;
		}

		// and connect it to the given address (port to bind socket to will be chosen randomly)
		if (connect(socket_fd, p->ai_addr, p->ai_addrlen) == -1) {
			close(socket_fd);
			perror("Failed to connect socket to given IP");
			continue;
		}
		return socket_fd;
	}		
	fprintf(stderr, "Client failed to connect to anything\n");
	exit(1);
}
