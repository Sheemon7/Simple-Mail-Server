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
#include "messages.h"

#include "password/login_helper.h"

#define MAXDATASIZE 100

void run_server(int listener) {
	char buf[256];    // buffer for client data
    char username[100]; // buffer for username
    char password[100]; // buffer for passwords
    char remoteIP[INET6_ADDRSTRLEN];
    int nbytes, userlen, fdmax, newfd;
    fd_set master;    // master file descriptor list
    fd_set read_fds;  // temp file descriptor list for select()
    FD_ZERO(&master);    // clear the master and temp sets
    FD_ZERO(&read_fds);
	login_helper *logins = init(); // init login structure
    // add the listener to the master set
    FD_SET(listener, &master);
    struct sockaddr_storage remoteaddr; // client address
    socklen_t addrlen;
    logged_user *usr;
    char msg[256];
    messages_saver *mssgs = init_saver(); // init saving messages

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
                        printf("selectserver: new connection from %s on "
                            "socket %d\n",
                            inet_ntop(remoteaddr.ss_family,
                                get_in_addr((struct sockaddr*)&remoteaddr),
                                remoteIP, INET6_ADDRSTRLEN),
                            newfd);
 
                        // authentication
		           		if((nbytes = get_message(newfd, username, &master, logins)) <= 0) {
                            printf("Kurva\n");
		           			continue;
		           		}
		           		username[nbytes-1] = '\0'; //Mistake nbytes index -1
                        printf("Username is %s\n",username);

                        if((nbytes = get_message(newfd, password, &master, logins)) <= 0) {
                        	continue;
                        }
                        password[nbytes-1] = '\0';
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

                        //TODO - send him all available messages
                        // while (get_saved_message(mssgs, username, &msg) == 0) {
                        //     // send it.
                        // }
                    }
                } else {
                    // handle data from a client
                    if ((nbytes = get_message(i, buf, &master, logins)) <= 0) {
                        continue;
                    } else {
                    	printf("NBYTES: %d\n", nbytes);
                    	buf[nbytes-1] = '\0';
                        printf("Received a new message: %s\n", buf);
                    	
                    	// replace : with \0
                    	// for(int i = 0; i < nbytes; ++i) {
                    	// 	if (buf[i] == ':') {
                    	// 		// buf[i] = '\0';
                    	// 		userlen = i;
                    	// 		break;
                    	// 	}

                    	// }
                    	// printf("Userlen: %d\n", userlen);
                    	// printf("NBYTES: %d\n", nbytes);

                        // zkontrolovat availability, jinak save, a poslat kody
                        //Potvrzovaci protokol

                        // we got some data from a client
                        for(int j = 0; j <= fdmax; j++) {
                            // send to everyone!
                            if (FD_ISSET(j, &master)) {
                                // except the listener and ourselves
                                if (j != listener && j != i) {
                                    if (send(j, buf, nbytes, 0) == -1) {
                                        perror("send");
                                    }

                                }

                             //    if (j != listener && j != i) {
	                            //     usr = get_user_fd(logins, j);

	                            //     if (usr != NULL && strcmp(usr->name, buf) == 0) {
	                            //     	//printf("Username: %s\nBuffer: %s\n", usr->name, buf);
	                            //     	printf("Sending\n");
	                            //     	if (send(j, buf+userlen+1, nbytes-userlen, 0) == -1) {
	                            //             perror("send");
	                            //         }
	                            //     } else {
	                            //     	printf("User with nick : %s not found", buf);
	                            //     }
	                            // }
                            }
                        }
                    }
                }
            }
        }
        destroy(&logins);
        destroy_saver(&mssgs);
        logins = NULL;
        mssgs = NULL;
    }
}
