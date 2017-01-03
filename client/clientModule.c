#include <arpa/inet.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include "clientModule.h"


#define MAXDATASIZE 100 // max number of bytes we can get at once 
void sendMsg(int socket_fd, char msg[], char login[]);
void receiveMsg(int socket_fd, char msg[]);
int checkUsername(int socket_fd, char login[]);
int checkPassword(int socket_fd, int tries, char login[]);
int endCommunication(int socket_fd);

int clientModule(int socket_fd,char login[]){
	int len, bytes_send;
	char buf[MAXDATASIZE];
	char msgToSend[MAXDATASIZE];
	int usernameResult = checkUsername(socket_fd, login);
	// int passwordResult = checkPassword(socket_fd, 0, login);
	// perror("Ahoj");

	if (!fork()) {
		// this is the child process which waits what user inputs
		int ongoingCom = 1;
		char userInput[MAXDATASIZE];
		while(ongoingCom){
			scanf("%s",userInput);
			if(strcmp(userInput, "quit") == 0){
				endCommunication(socket_fd);
			}else{
				sendMsg(socket_fd, userInput, login);
			}
			// printf("%s\n", userInput);
		}	
    }
    printf("Konec");
    waitpid(-1, NULL, 0);
    //This process waits if server has sended some messages
    


 

}

void sendMsg(int socket_fd, char msg[], char login[]){
	//Custom function for sending
	//Here will be implemented the simulation of the lost messages;
	char msgToSend[MAXDATASIZE];
	strcpy(msgToSend, login);
	strcat(msgToSend, ":");
	strcat(msgToSend, msg);
	// sprintf(socket_fd, "%s", msgToSend);
	int len = strlen(msgToSend);
	send(socket_fd, msgToSend, len, 0); 
	printf("%s\n",msgToSend);
}

void receiveMsg(int socket_fd, char msg[]){
	char buf[MAXDATASIZE];
	//Custom function for receiving
	//Here will be implemented the simulation of the lost messages
	// sscanf(socket_fd, buf);
	recv(socket_fd, buf, MAXDATASIZE, 0);
	printf("%s", buf);
}

int checkUsername(int socket_fd, char login[]){
	char buf[MAXDATASIZE];
	//send username
	sendMsg(socket_fd, login, login);
	//receive
	receiveMsg(socket_fd, buf);

	if(strcmp(buf, "0") == 0){//Right Username which is already created
		printf("User exists\n");
		printf("Please Enter your password\n");
		return 0;
	}else if(strcmp(buf, "1") == 0){//New User, created
		printf("User does not exist");
		endCommunication(socket_fd);
		return 1;
	}
}


/*
Password codes
0 - right password
-1 - error
1 - wrong password
2 - number of tries exceeded

*/
int checkPassword(int socket_fd, int tries, char login[]){
	char buf[MAXDATASIZE];
	scanf("%s", buf);
	sendMsg(socket_fd, buf, login);
	// sscanf(socket_fd, "%s", buf);
	recv(socket_fd, buf, MAXDATASIZE, 0);
	if(strcmp(buf, "0") == 0){//right password
		return 0;
	}else if (strcmp(buf, "0") == 1){ //wrong password
		return checkPassword(socket_fd, tries+1, login);
	}else if (tries >= 5){
		printf("Too many Attempts please connect again");
		endCommunication(socket_fd);
		return 2;
	}else{
		endCommunication(socket_fd);
		return -1;
	}
}

int endCommunication(int socket_fd){
	//Not important now can also call again main function
	close(socket_fd);
	exit(0);
	
}