/*
** client.c -- a stream socket client demo
*/

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <netdb.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <stdio.h>
#include <sys/types.h>
#include <signal.h>
#include <arpa/inet.h>

#include "sendall.h"

// #define PORT "9034" // the port client will be connecting to 

#define MAXDATASIZE 100 // max number of bytes we can get at once 

int sendMessageToUser(int sockfd, int pipe);
void set_args(char *argv[],char server_ip[], char server_port[], char login[]);
int sendMessage(int sockfd, char msg[], int pipe);
int recvMessage(int sockfd, char buf[], int pipe);

// get sockaddr, IPv4 or IPv6:
void *get_in_addr(struct sockaddr *sa)
{
    if (sa->sa_family == AF_INET) {
        return &(((struct sockaddr_in*)sa)->sin_addr);
    }

    return &(((struct sockaddr_in6*)sa)->sin6_addr);
}

int main(int argc, char *argv[])
{
    int sockfd, numbytes;  
    char buf[MAXDATASIZE];
    struct addrinfo hints, *servinfo, *p;
    int rv;
    char s[INET6_ADDRSTRLEN];

    if (argc != 4) {
        fprintf(stderr,"usage: client hostname, server port, login\n");
        exit(1);
    }

    char login[MAXDATASIZE];
    char server_port[MAXDATASIZE];
    char server_ip[MAXDATASIZE];
    
    set_args(argv, server_ip, server_port, login);


    memset(&hints, 0, sizeof hints);
    hints.ai_family = AF_UNSPEC;
    hints.ai_socktype = SOCK_STREAM;

    if ((rv = getaddrinfo(server_ip, server_port, &hints, &servinfo)) != 0) {
        fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(rv));
        return 1;
    }

    // loop through all the results and connect to the first we can
    for(p = servinfo; p != NULL; p = p->ai_next) {
        if ((sockfd = socket(p->ai_family, p->ai_socktype,
                p->ai_protocol)) == -1) {
            perror("client: socket");
            continue;
        }

        if (connect(sockfd, p->ai_addr, p->ai_addrlen) == -1) {
            close(sockfd);
            perror("client: connect");
            continue;
        }

        break;
    }

    if (p == NULL) {
        fprintf(stderr, "client: failed to connect\n");
        return 2;
    }

    inet_ntop(p->ai_family, get_in_addr((struct sockaddr *)p->ai_addr),
            s, sizeof s);
    printf("client: connecting to %s\n", s);

    freeaddrinfo(servinfo); // all done with this structure

    //Client Code
    //Create Pipe
    int pd[2];
    
    if(pipe(pd) == -1){
        perror("Pipe not crated");
    }

    //Login Session
    int loginLength = strlen(login);
    // sendall(sockfd)
    if (sendall(sockfd, login, &loginLength) == -1) {
        perror("sendall");
        printf("We only sent %d bytes because of the error!\n", loginLength);
    } 
    

    printf("Logging as %s\n",login);
    char password[MAXDATASIZE];
    printf("Enter password : ");
    fgets(password, MAXDATASIZE-1, stdin);
    int passwordLength = strlen(password);
	password[passwordLength-1] = '\0';
    
    if (sendall(sockfd, password, &passwordLength) == -1) {
        perror("sendall");
        printf("We only sent %d bytes because of the error!\n", passwordLength);
    } 
    


    //Receive Server Response
    if ((numbytes = recv(sockfd, buf, MAXDATASIZE-1, 0)) == -1) {
        perror("recv");
        exit(1);
    }
    buf[numbytes] = '\0';

    if(strcmp(buf,"0") == 0){
        printf("You have succesfully logged in \n");
    }else if(strcmp(buf,"1") == 0){
        printf("Wrong Password");
    }    

    //Here we will create fork
    int pid = fork();
    if(pid == 0){
        //Child Process
        //Manage pipe
        if(close(pd[0])==-1){
            perror("Close of pipe failed");
        }

        //Receive Message
        while(1){
    		recvMessage(sockfd, buf, pd[1]);
            int bufLength = strlen(buf);
            printf("client: received '%s'\n",buf);
             
        }
        
    }
    //Pipe from receving messages to sending messages
    //Only 100 expected as input from pipe
    //When 100 is encountered we know the message was delivered succesfully
    if(close(pd[1])==-1){
        perror("Close of pipe failed");
    } //closes pipe => fork 2 stops

    
    printf("Send quit to exit\n");
    int comOn = 1;
    while(comOn){
        comOn = sendMessageToUser(sockfd, pd[0]);
    }

    kill(pid, SIGTERM);

    //Termination of client
    close(sockfd);

    return 0;
}

int recvMessage(int sockfd, char buf[], int pipe){
	int numbytes;	
	if ((numbytes = recv(sockfd, buf, MAXDATASIZE-1, 0)) == -1) {
        perror("recv");
        exit(1);
    }         
    buf[numbytes] = '\0';

    //Sever message delivered
    if(strcmp(buf,"100") == 0){
        write(pipe, "100", 4);
    }
}

int sendMessage(int sockfd, char msg[], int pipe){
	char buf[MAXDATASIZE];
	int msgLength = strlen(msg);
	int msgReceived = 0;

	while(msgReceived == 0){
		printf("Trying to send message %s\n", msg);

        if (sendall(sockfd, msg, &msgLength) == -1) {
            perror("sendall");
            printf("We only sent %d bytes because of the error!\n", msgLength);
        } 

	    sleep(1);

	    read(pipe, buf, MAXDATASIZE);

	    if(strcmp(buf,"100") == 0){
	    	printf("Message Succesfully received by server \n");
	    	msgReceived = 1;
	    }else{
	    	printf("%s\n",buf);
	    }

	}
}

int sendMessageToUser(int sockfd, int pipe){
    char msg[MAXDATASIZE];
    printf("Message: ");
    fgets(msg, MAXDATASIZE-1, stdin);
    int msgLength = strlen(msg);

	msg[msgLength-1] = '\0';

    if(strcmp(msg,"quit") == 0){
        return 0;
    }

    sendMessage(sockfd, msg, pipe);
    return 1;
}

void set_args(char *argv[],char server_ip[], char server_port[], char login[]) {
    strcpy(server_ip, argv[1]);
    strcpy(server_port, argv[2]);
    strcpy(login, argv[3]);
}
