#ifndef HELPER_FUNCTIONS_H
#define HELPER_FUNCTIONS_H

#include "password/login_helper.h"

int bind_server_to_port(struct addrinfo *servinfo);

void *get_in_addr(struct sockaddr *sa);

int get_message(int fd, char *buf, fd_set *master, login_helper *h);

#endif
