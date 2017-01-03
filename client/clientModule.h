#ifndef CLIENTMODULE_H
#define CLIENTMODULE_H

int clientModule(int socket_fd,char login[]);
void sendMsg(int socket_fd, char msg[], char login[]);
void receiveMsg(int socket_fd, char msg[]);
int checkUsername(int socket_fd, char login[]);
int checkPassword(int socket_fd, int tries, char login[]);
int endCommunication(int socket_fd);


#endif