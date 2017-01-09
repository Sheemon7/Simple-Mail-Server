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
#include <sys/wait.h>
#include <time.h>

#include "sendall.h"

#define MAXDATASIZE 100 // max number of bytes we can get at once 

int sendMessageToUser(int sockfd, int pipe);
void set_args(char *argv[],char server_ip[], char server_port[], char login[]);
int sendMessage(int sockfd, char msg[], int pipe);
int recvMessage(int sockfd, char buf[], int pipe);
// void signalHandler(int signum);

int comOn = 1;

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

    //array of messages
    int maxNumOfMsg = 10;
    char messages[maxNumOfMsg][MAXDATASIZE];

    //Create Pipe
    int pd[2];
    

    if(pipe(pd) == -1){
        perror("Pipe not crated");
    }

    //Here we will create fork
    int pid = fork();
    if(pid == 0){
        //Server Input
        if(close(pd[0])==-1){
            perror("Close of pipe failed");
        }

        //Receive Message
        while(comOn){
            int recvLen;
    		if((recvLen =recvMessage(sockfd, buf, pd[1]))<= 0){
                write(pd[1],"quit\n",6);
                break;
            }
            buf[recvLen] = '\0';
            if(strcmp(buf,"0\n")==0){
                printf("Right Password\n");
                continue;
            }

            
            if(strcmp(buf,"300\n") == 0){
                printf("MESSAGE_DELIVERED\n");
                write(pd[1],"300\n",5);
                continue;
            }

            if(strcmp(buf,"301\n") == 0){
                printf("USER_NOT_EXISTS_CODE\n");
                continue;
            }

            if(strcmp(buf,"302\n") == 0){
                printf("MESSAGE_SAVED_CODE\n");
                continue;
            }

            if(strcmp(buf,"303\n") == 0){
                printf("MESSAGE_NOT_SAVED_CODE\n");
                continue;
            }


            if(strcmp(buf,"1\n")==0){
                printf("Wrong Password\nLog again please\n");
                write(pd[1],"quit\n",6);
                break;
            }
            printf("client received %s",buf);
        }
        exit(0);
    }
    int pid2 = fork();
    if(pid2 == 0){
        //User Child
        // signal(SIGKILL, signalHandler);
        //Here will be user login
        char loginToSend[MAXDATASIZE];
        strcpy(loginToSend, login);
        int loginLength = strlen(login);

        int loginToSendLength = strlen(loginToSend);
        loginToSend[loginToSendLength] = '\n';
        loginToSendLength++;

        

        printf("Logging as %s\n",login);
        char password[MAXDATASIZE];
        printf("Enter password : ");
        fgets(password, MAXDATASIZE-1, stdin);
        int passwordLength = strlen(password);

        write(pd[1],loginToSend, loginToSendLength);
        sleep(1);
        write(pd[1],password,passwordLength);





        //User Input
        printf("Send quit to exit\n");
        printf("Send messages in format User:Message\n");

        if(close(pd[0])==-1){
            perror("Close of pipe failed");
        }
        while(comOn){
            char msgLog[MAXDATASIZE];
            char msg[MAXDATASIZE];
            fgets(msg, MAXDATASIZE-1, stdin);
            int msgLength = strlen(msg);
            int containsDelimiter = 0;
            for (int i = 0; i < msgLength; ++i){
                if(msg[i] == ':'){
                    if(containsDelimiter){
                        printf("Use just one delimiter\n");
                        containsDelimiter = 0;
                        break;
                    }
                    if(i==0){
                        printf("Please enter username \n");
                        containsDelimiter = 0;
                        break;
                    }

                    if(i==msgLength-2){
                        printf("Message is empty\n");
                        containsDelimiter = 0;
                        break;
                    }
                    containsDelimiter = 1;
                }
            }

            if(containsDelimiter == 0 && strcmp(msg,"quit\n") != 0){
                printf("Send messages in format User:Message\n");
                continue;
            }
            
            write(pd[1],msg, msgLength+1);
        }
        exit(0);
    }


    if(close(pd[1])==-1){
        perror("Close of pipe failed");
    }

    
    
    char msg[MAXDATASIZE];
    int msgLength = 0;
    //Message Data Structures
    int sendingMsg = 0;
    
    int nextMsgSave = 0;
    int currMsgSending = 0;
    int numMsg = 0;
    int pid3;
    // srand(time(NULL));

    while(comOn){
        read(pd[0], msg, MAXDATASIZE-1);
        //client terminations
        if(strcmp(msg,"quit\n") == 0){
            comOn = 0;
            // kill(pid, SIGKILL);
            // kill(pid2, SIGKILL);
            break;
        }else if(strcmp(msg,"300\n") == 0){
            sendingMsg = 0;
            kill(pid3,SIGKILL);
        }else{
            if(numMsg <= maxNumOfMsg){
                strcpy(messages[nextMsgSave], msg);
                nextMsgSave++;
                if(nextMsgSave == 10){
                    nextMsgSave = 0;
                }
                numMsg++;
            }else{
                printf("Too many msgs");
            }
        }

        int msgLength = strlen(msg);
        //Protokol na posilani vice zprav a ukladani zprav
        
        if(numMsg>0 && comOn){
            if(!sendingMsg){ // rovna se 0
                msgLength = strlen(messages[currMsgSending]);
                pid3 = fork();
                if(pid3==0){
                    while(1){
                        // printf("ERROR in connection\n");
                        srand(time(NULL));
                        if(rand()%4){
                            sendMessage(sockfd, messages[currMsgSending], msgLength);
                        }else{
                            printf("ERROR in connection\n");
                        }
                        sleep(1);
                    }
                }
                numMsg--;
                currMsgSending++;
                if(currMsgSending == 10){
                    currMsgSending = 0;
                }
                sendingMsg = 1;
                // printf("%d\n", currMsgSending);
                


            }
        }
        
    }

    kill(pid, SIGKILL);
    kill(pid2, SIGKILL);
    
    waitpid(-1, NULL, 0);
    //Termination of client
    close(sockfd);

    return 0;
}



int recvMessage(int sockfd, char buf[], int pipe){
	int nbytes;	
    if ((nbytes = get_message(sockfd, buf)) <= 0) {
        printf("\nServer hung up\n");
        return nbytes;
    }
}

int sendMessage(int sockfd, char msg[], int pipe){
	char buf[MAXDATASIZE];
	int msgLength = strlen(msg);
	int msgReceived = 0;

    if (sendall(sockfd, msg, &msgLength) == -1) {
        perror("sendall");
        printf("We only sent %d bytes because of the error!\n", msgLength);
    } 

}

void set_args(char *argv[],char server_ip[], char server_port[], char login[]) {
    strcpy(server_ip, argv[1]);
    strcpy(server_port, argv[2]);
    strcpy(login, argv[3]);
}
