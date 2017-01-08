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

    //Create Pipe
    int pd[2];
    

    if(pipe(pd) == -1){
        perror("Pipe not crated");
    }

    //Here we will create fork
    int pid = fork();
    if(pid == 0){
        //Server Input
        // signal(SIGKILL, signalHandler);

        if(close(pd[0])==-1){
            perror("Close of pipe failed");
        }

        //Receive Message
        while(comOn){
    		if(recvMessage(sockfd, buf, pd[1])<= 0){
                while(comOn){
                    write(pd[1],"quit\n",5);
                }
                break;
            }

            if(strcmp(buf,"0\n")==0){
                printf("Right Password\n");
                continue;
            }
            if(strcmp(buf,"1\n")==0){
                printf("Wrong Password\nLog again please\n");
                write(pd[1],"quit\n",5);
                break;
            }

            fflush(stdout);
            printf("client: received %s",buf);
            fflush(stdout);
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
	    printf("WARNING: Pokud vase zpravy nebudou obsahovat objektivni informace, nebudou odeslany!!!\n");
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
    while(comOn){
        read(pd[0], msg, MAXDATASIZE-1);

        //client terminations
        if(strcmp(msg,"quit\n") == 0){
            fflush(stdout);
            printf("Cus\n");
            comOn = 0;
            break;
        }
        int msgLength = strlen(msg); //znak '\0' se neposle
        sendMessage(sockfd, msg, msgLength);
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

    //Sever message delivered
    // if(strcmp(buf,"100") == 0){
        // write(pipe, "100", 4);
    // }
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

// void signalHandler(int signum){
//     if(signum == SIGUSR1){
// //     //     exit(0);
// //     //     comOn = 0;
//     }
//     if(signum == SIGKILL){
//         printf("\nSigkill used \n");
//         exit(0);
//     }
// }

void set_args(char *argv[],char server_ip[], char server_port[], char login[]) {
    strcpy(server_ip, argv[1]);
    strcpy(server_port, argv[2]);
    strcpy(login, argv[3]);
}
