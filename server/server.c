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

#define MAXDATASIZE 256
#define MAXWORDSIZE 100

#define CORRECT_PASSWORD_CODE "0\n"
#define WRONG_PASSWORD_CODE "1\n"

#define MESSAGE_PROPERLY_SENT_CODE "300\n"
#define USER_NOT_EXISTS_CODE "301\n"
#define MESSAGE_SAVED_CODE "302\n"
#define MESSAGE_NOT_SAVED_CODE "303\n"

#define MESSAGE_RECEIVED_CODE "100\n"

#define CAPACITY 5

void run_server(int server_fd) {      
    char remoteIP[INET6_ADDRSTRLEN];
    int fdmax, newfd; // descriptors
    fd_set master;    // master file descriptor list
    fd_set read_fds;  // temp file descriptor list for select()
	
    struct sockaddr_storage remoteaddr; // client address
    socklen_t addrlen;
    
    // keep track of the biggest file descriptor
    fdmax = server_fd; // so far, it's this one
    FD_ZERO(&master);    // clear the master and temp sets
    FD_ZERO(&read_fds);
    FD_SET(server_fd, &master); // add the server_fd to the master set

    char buf[MAXDATASIZE];          // buffer for messages
    char msg[MAXDATASIZE];          // aux buffer for resending
    char username[MAXWORDSIZE];     // buffer for username
    char password[MAXWORDSIZE];     // buffer for passwords
    int msg_len, user_len, nbytes, msg_start, ret_code;

    login_helper *logins = init(); // login-handling struct
    logged_user *usr, *rec, *sender; // connected user struct
    messages_saver *mssgs = init_saver(CAPACITY);   // message-saving struct

    /* server MAIN LOOP */
    while(1) {
        read_fds = master;
        if (select(fdmax + 1, &read_fds, NULL, NULL, NULL) == -1) {
            perror("select");
            exit(4);
        }
        
        // run through the existing connections looking for data to read
        for(int i = 0; i <= fdmax; i++) {
            // print(logins);
            if (FD_ISSET(i, &read_fds)) {
                if (i == server_fd) { // server has something to read from
                    addrlen = sizeof(remoteaddr);
                    newfd = accept(server_fd, (struct sockaddr *)&remoteaddr, &addrlen);
                    if (newfd == -1) {
                        perror("Couldn't accept new connection");
                    } else {
                        FD_SET(newfd, &master); // add to master set
                        if (newfd > fdmax) {    // keep track of the max
                            fdmax = newfd;
                        }
                        printf("New connection from %s on socket %d\n",
                            inet_ntop(remoteaddr.ss_family, get_in_addr((struct sockaddr*)&remoteaddr),
                                remoteIP, INET6_ADDRSTRLEN), newfd);
 
                        // authentication
		           		if((nbytes = get_message(newfd, username, MAXWORDSIZE, &master, logins)) <= 0) { 
		           			continue; // client disconnected
		           		}
		           		username[nbytes-1] = '\0'; // append '\0' at the end of the message
                        // TODO - move to get_message
                        printf("Username is %s\n",username);

                        //Promin testovani
                        msg_len = 4;
                        sendall(newfd, MESSAGE_PROPERLY_SENT_CODE, &msg_len, &master, logins);
                        sleep(1);

                        if((nbytes = get_message(newfd, password, MAXWORDSIZE, &master, logins)) <= 0) {
                        	continue; // client disconnected
                        }
                        password[nbytes-1] = '\0'; // Uz spraveno
						printf("Password is %s\n", password);

                        //Promin testovani
                        msg_len = 4;
                        sendall(newfd, MESSAGE_PROPERLY_SENT_CODE, &msg_len, &master, logins);
                        sleep(1);

                        msg_len = 2;
                        if (add_user(logins, newfd, username, strlen(username), password, strlen(password)) == -1) {
                        	printf("User %s sent wrong password, not allowing connection\n", username);
                            sendall(newfd, WRONG_PASSWORD_CODE, &msg_len, &master, logins);
                        } else {
                        	printf("User %s sent correct password, allowing connection\n", username);
                            sendall(newfd, CORRECT_PASSWORD_CODE, &msg_len, &master, logins);
                            sleep(1);
                            printf("Sending saved messages to new user %s\n", username);

                            while (get_saved_message(mssgs, username, msg) == 0) {
                                printf("Sending message %s to %s\n", msg, username);
                                msg_len = strlen(msg);
                                if (sendall(newfd, msg, &msg_len, &master, logins) == -1) {
                                    fprintf(stderr, "Sent only %d bytes of message!", msg_len);    
                                }
                                sleep(1);
                                printf("%d bytes was send\n", msg_len);
                            }
                        }
                    }
                } else { // client has something to read from
                    if ((nbytes = get_message(i, buf, MAXDATASIZE, &master, logins)) <= 0) {
                        continue;
                    } else {
                    	buf[nbytes] = '\0'; // TODO - move to get_message
                        printf("Received a new message: %s", buf);

                        // replace ':'' with '\0'
                    	for(int k = 0; k < nbytes; ++k) {
                    		if (buf[k] == ':') {
                    			buf[k] = '\0';
                    			msg_start = k + 1;
                    			break;
                    		}
                    	}

                        print(logins);

                        sender = get_user_fd(logins, i, &ret_code);
                        if (sender->last_msg != NULL && strcmp(sender->last_msg, &buf[msg_start]) == 0) {
                            if (rand() % 4 == 0) {
                                msg_len = 4;
                                sendall(i, MESSAGE_PROPERLY_SENT_CODE, &msg_len, &master, logins);
                            }
                            continue; // received another message
                        }

                        // save message
                        if (sender->last_msg != NULL) {
                            free(sender->last_msg);
                        }
                        sender->last_msg = malloc(sizeof(char) * nbytes);
                        strcpy(sender->last_msg, &buf[msg_start]);

                        msg_len = 4;
                        rec = get_user_name(logins, buf, &ret_code);
                        if (rec == NULL) {
                            printf("User %s doesn't exist\n", buf);
                            sendall(i, USER_NOT_EXISTS_CODE, &msg_len, &master, logins);
                            sleep(1);
                        } else if (ret_code == -1) {
                            printf("Receiver exists, but is not available\n");
                            if (save_message(mssgs, buf, buf + msg_start) == 1) { // full messages
                                printf("Message was not saved due to capacity reasons\n");
                                sendall(i, MESSAGE_NOT_SAVED_CODE, &msg_len, &master, logins);
                                sleep(1);
                            } else {
                                printf("Message was saved\n");
                                sendall(i, MESSAGE_SAVED_CODE, &msg_len, &master, logins);    
                                sleep(1);
                            }
                        } else {
                            printf("Message sent\n");
                            // msg_len = 4;
                            // sendall(i, "100\n" , &msg_len, &master, logins);
                            // sleep(1);
                            sendall(i, MESSAGE_PROPERLY_SENT_CODE, &msg_len, &master, logins);
                            sleep(1);
                            // msg_len = nbytes - msg_start;
                            msg_len = nbytes - msg_start;
                            sendall(rec->fd, buf + msg_start, &msg_len, &master, logins);
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
