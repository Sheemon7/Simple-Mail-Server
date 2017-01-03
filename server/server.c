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
#include <string.h>

#include "password/password_handle.h"

static char *LOGIN_MSG = "Please login\n";
static int LOGIN_LEN = 14;
static char *PASSWORD_MSG = "Password...\n";
static int PASSWORD_LEN = 13;
static char *SUCCES_MSG = "Succesfully logged in\n";
static int SUCCES_LEN = 23;

int accept_new_connection(int listener_fd);
int get_login(int client_fd, char *buff);
int get_password(int client_fd, char *buff);

int run_server(int socket_fd) {
	char buff[100];
	int max_fd = socket_fd, new_fd;
	fd_set master, read_fds;
	FD_ZERO(&master);
	FD_ZERO(&read_fds); // set to zero (empty)
	FD_SET(socket_fd, &master); // add listener fd to set
	
	char logins[100][100];
	char password[100];
	char **messages[100];
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
					new_fd = accept_new_connection(socket_fd);
					if (get_login(new_fd, logins[new_fd]) 
						&& get_password(new_fd, password) 
						&& authenticate(logins[new_fd], password)) {
						if (send(new_fd, SUCCES_MSG, SUCCES_LEN, 0) == -1) {
							perror("send");
						}	
					
						if (new_fd > max_fd) { max_fd = new_fd; }
						FD_SET(new_fd, &master);	
					} else {
						close(new_fd);
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

int accept_new_connection(int client_fd) {
	struct sockaddr_storage client_addr;
	socklen_t addr_len = sizeof(client_addr);
	int new_fd = accept(client_fd, (struct sockaddr *)&client_addr, &addr_len);
	
	if (new_fd == -1) {
		perror("Failed to accept a connection");
	} else {
		new_fd = dup(new_fd);
	}
	return new_fd;
}

int get_login(int client_fd, char *login_buff) {
	if (send(client_fd, LOGIN_MSG, LOGIN_LEN, 0) == -1) {
		perror("Error sending login");
	}

	int nbytes;
	if (nbytes = recv(client_fd, login_buff, sizeof(login_buff), 0) <= 0) {
	// error or connection closed by client
		if (nbytes == 0) {
			printf("Select server : socket %d hung up\n", client_fd);
		} else {
			perror("Error in receiving login");
		}
		return -1;
	}
}

int get_password(int client_fd, char *password_buff) {
	if (send(client_fd, PASSWORD_MSG, PASSWORD_LEN, 0) == -1) { 
		perror("Error sending password");
	}

	int nbytes;
	if (nbytes = recv(client_fd, password_buff, sizeof(password_buff), 0) <= 0) {
	// error or connection closed by client
		if (nbytes == 0) {
			printf("Select server : socket %d hung up\n", client_fd);
		} else {
			perror("Error in receiving password");
		}
		return -1;
	}
 }
