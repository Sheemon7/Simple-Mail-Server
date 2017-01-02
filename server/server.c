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

int accept_new_connection(int listener_fd, fd_set *master);

int run_server(int socket_fd) {
	int max_fd = socket_fd, new_fd;
	fd_set master, read_fds;
	FD_ZERO(&master);
	FD_ZERO(&read_fds); // set to zero (empty)
	FD_SET(socket_fd, &master); // add listener fd to set
	

	while(1) {
		read_fds = master; // copy it
		if (select(max_fd + 1, &read_fds, NULL, NULL, NULL) == -1) { // we are only interested in read echos
			perror("Select failed");
			exit(1);
		}	
		
		// run through existing connections looking for data to read
		for(int i = 0; i <= max_fd; ++i) {
			if(FD_ISSET(i, &read_fds)) {
				if (i == socket_fd) { // listener reads new connection
					new_fd = accept_new_connection(socket_fd, &master);
					if (new_fd > max_fd) {
						max_fd = new_fd;			
					}
				} else { // new data from client
					
				//debug
					int nbytes;
					char buf[100];
					int j;	
				    if ((nbytes = recv(i, buf, sizeof buf, 0)) <= 0) {
                		       // got error or connection closed by client
                        		if (nbytes == 0) {
                            		// connection closed
                            			printf("selectserver: socket %d hung up\n", i);
                        		} else {
                            			perror("recv");
                        		}
                        		close(i); // bye!
                        		FD_CLR(i, &master); // remove from master set
                    		} else {
                        	// we got some data from a client
                        		for(j = 0; j <= max_fd; j++) {
                            			// send to everyone!
                            			if (FD_ISSET(j, &master)) {
                                		// except the listener and ourselves
                                			if (j != socket_fd && j != i) {
                                    				if (send(j, buf, nbytes, 0) == -1) {
                                        				perror("send");
                                    				}
                                			}
                            			}
                        		}
                    		}
						}
				}		
			}	
		}
}

int accept_new_connection(int listener_fd, fd_set *master) {
	struct sockaddr_storage client_addr;
	socklen_t addr_len = sizeof(client_addr);
	int new_fd = accept(listener_fd, (struct sockaddr *)&client_addr, &addr_len);
	
	if (new_fd == -1) {
		perror("Failed to accept a connection");
	} else {
		FD_SET(new_fd, master);
	}
	return new_fd;
}
