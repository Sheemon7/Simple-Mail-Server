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
#include <arpa/inet.h>
#include <signal.h>

#include "server.h"
#include "helper_functions.h"

#define BACKLOG 10     // how many pending connections queue will hold
#define MAX_PORT_LENGTH 10


int main(int argc, char *argv[]) {   
    int listener;     // listening socket descriptor
    int newfd;        // newly accept()ed socket descriptor
    int yes=1;        // for setsockopt() SO_REUSEADDR, below
    int rv;

    struct addrinfo hints, *ai, *p;

    if (argc != 2) {
    	fprintf(stderr, "Usage: ./server port\n");
    	exit(1);
    }

    // get us a socket and bind it
    memset(&hints, 0, sizeof(hints));
    hints.ai_family = AF_UNSPEC;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_flags = AI_PASSIVE;
    if ((rv = getaddrinfo(NULL, argv[1], &hints, &ai)) != 0) {
        fprintf(stderr, "selectserver: %s\n", gai_strerror(rv));
        exit(1);
    }
    
    listener = bind_server_to_port(ai);

    freeaddrinfo(ai); // all done with this

    // listen
    if (listen(listener, 10) == -1) {
        perror("listen");
        exit(3);
    }

    run_server(listener);   
    close(listener);
    return 0;
}
