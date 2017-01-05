/*
** selectserver.c -- a cheezy multiperson chat server
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>

#include "helper_functions.h"

#include "password/login_helper.h"

#define MAXDATASIZE 100

// get sockaddr, IPv4 or IPv6:
int get_message(int fd, char *buf, fd_set *master, login_helper *h);

void run_server(int listener) {
	char buf[256];    // buffer for client data
    char username[100]; // buffer for username
    char password[100]; // buffer for passwords
    char remoteIP[INET6_ADDRSTRLEN];
    int nbytes, fdmax, newfd;
    fd_set master;    // master file descriptor list
    fd_set read_fds;  // temp file descriptor list for select()
    FD_ZERO(&master);    // clear the master and temp sets
    FD_ZERO(&read_fds);
	login_helper *logins = init(); // init login structure
    // add the listener to the master set
    FD_SET(listener, &master);
    struct sockaddr_storage remoteaddr; // client address
    socklen_t addrlen;

    // keep track of the biggest file descriptor
    fdmax = listener; // so far, it's this one

    for(;;) {
        read_fds = master; // copy it
        if (select(fdmax+1, &read_fds, NULL, NULL, NULL) == -1) {
            perror("select");
            exit(4);
        }
        
        // run through the existing connections looking for data to read
        for(int i = 0; i <= fdmax; i++) {
        	print(logins);
            if (FD_ISSET(i, &read_fds)) { // we got one!!
                if (i == listener) {
                    addrlen = sizeof(remoteaddr);
                    newfd = accept(listener,
                        (struct sockaddr *)&remoteaddr,
                        &addrlen);
                    if (newfd == -1) {
                        perror("accept");
                    } else {
                        FD_SET(newfd, &master); // add to master set
                        if (newfd > fdmax) {    // keep track of the max
                            fdmax = newfd;
                        }
                        /*printf("selectserver: new connection from %s on "
                            "socket %d\n",
                            inet_ntop(remoteaddr.ss_family,
                                get_in_addr((struct sockaddr*)&remoteaddr),
                                remoteIP, INET6_ADDRSTRLEN),
                            newfd);*/
 
                        // authentication
                   		if(get_message(newfd, username, &master, logins) <= 0) {
                   			continue;
                   		}
                        printf("Username is %s\n",username);

                        if(get_message(newfd, password, &master, logins) <= 0) {
                        	continue;
                        }
						printf("Password is %s\n", password);

                        if (add_user(logins, newfd, username, strlen(username), password, strlen(password)) == -1) {
                        	printf("Incorrect password, not allowing connection\n");
                        	if (send(newfd, "1", 1, 0) == -1) {
                              perror("send");
                        	}
                        } else {
                        	printf("Correct password, allowing connection\n");
                        	if (send(newfd, "0", 1, 0) == -1) {
                              perror("send");
                        	}
                        }
                    }
                } else {
                    // handle data from a client
                    if (get_message(i, buf, &master, logins) <= 0) {
                        continue;
                    } else {
                        printf("Received a new message!");
                    	
                        //Potvrzovaci protokol

                        //Confirm Message - received it
                        if (send(i, "100", 4, 0) == -1) {
                            perror("send confirmation");
                        }


                        //

                        // // we got some data from a client
                        // for(j = 0; j <= fdmax; j++) {
                        //     // send to everyone!
                        //     if (FD_ISSET(j, &master)) {
                        //         // except the listener and ourselves
                        //         if (j != listener && j != i) {
                        //             if (send(j, buf, nbytes, 0) == -1) {
                        //                 perror("send");
                        //             }

                        //         }
                        //     }
                        // }
                    }
                }
            }
        }
    }
}
