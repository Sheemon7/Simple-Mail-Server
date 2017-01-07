#ifndef SENDALL_H
#define SENDALL_H

int sendall(int s, char *buff, int *len);
int get_message(int fd, char *buf);
int get_packet(int fd, char *buf);

#endif
