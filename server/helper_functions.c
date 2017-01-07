#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <string.h>

#include "password/login_helper.h"

int bind_server_to_port(struct addrinfo *servinfo) {
	struct addrinfo *p;
	int socket_fd;
	int yes = 1;
	for(p = servinfo; p != NULL; p = p->ai_next) {
		// try to create a socket
		if ((socket_fd = socket(p->ai_family, p->ai_socktype, p->ai_protocol)) == -1) {
			perror("Failed to create a socket");
			continue;
		}	

		// allow the programme to reuse the port if there is something still hanging
		if (setsockopt(socket_fd, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(yes)) == -1) {
			perror("Error setting socket options");
			exit(1);
		}

		// bind socket to its port
		if (bind(socket_fd, p->ai_addr, p->ai_addrlen) == -1) {
			close(socket_fd);
			perror("Failed to bind socket to port");
			continue;
		}
		return socket_fd;
	}
	fprintf(stderr, "Failed to bind to any port");
	exit(1);	
}

void *get_in_addr(struct sockaddr *sa) {
    if (sa->sa_family == AF_INET) {
        return &(((struct sockaddr_in*)sa)->sin_addr);
    }

    return &(((struct sockaddr_in6*)sa)->sin6_addr);
}

int get_packet(int fd, char *buf, int buf_size, fd_set *master, login_helper *h) {
	int nbytes;
	if ((nbytes = recv(fd, buf, 100, 0)) <= 0) { 
		// got error or connection closed by client
	    if (nbytes == 0) {
	        // connection closed
	        printf("selectserver: socket %d hung up\n", fd);
	    } else {
	        perror("recv");
	    }
		close(fd);
		FD_CLR(fd, master); // remove from master set
		remove_user_fd(h, fd);
	} else {
        //Confirm Message - received it
        // if (send(fd, "100\n", 4, 0) == -1) {
        //     perror("send confirmation");
        // }
	}
	return nbytes;
}

int get_message(int fd, char *buf, int buf_size, fd_set *master, login_helper *h) {
	int length = 0, nbytes;
	while(1) {
		nbytes = get_packet(fd, buf+length, buf_size - length, master, h);
		if (nbytes == 0) {
			break;
		} else if (buf[length + nbytes] = '\n') { // TODO WTF?
			length = length + nbytes;
			break;
		} else {
			length += nbytes;
		}
	}
	return length;
}

int sendall(int s, char *buf, int *len, fd_set *master, login_helper *h) {
    int total = 0;
    int bytesleft = *len;
    int n;
    char confirm[100];
    int nbytes;

    while(total < *len) {
        n = send(s, buf+total, bytesleft, 0);
        if (n == -1) { break; }
        // nbytes = get_message(s, confirm, master, h); // get confirmation
        // if (strcmp(confirm, "100") != 1) continue; // send again the same message
        total += n;
        bytesleft -= n;
    }

    *len = total; 

    return n == -1 ? -1 : 0;
} 
