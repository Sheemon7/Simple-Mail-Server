#include<sys/types.h>
#include<sys/socket.h>
#include <stdio.h>
#include <unistd.h>

int sendall(int s, char *buf, int *len) {
    int total = 0;
    int bytesleft = *len;
    int n;

    while(total < *len) {
        n = send(s, buf+total, bytesleft, 0);
        if (n == -1) { break; }
        total += n;
        bytesleft -= n;
    }

    *len = total; 

    return n == -1 ? -1 : 0;
}

int get_packet(int fd, char *buf) {
    int nbytes;
    // printf("%lu",sizeof(buf));
    if ((nbytes = recv(fd, buf, 100, 0)) <= 0) { //Here mistake in third argument of recv
        //Working but the constant 100 needs refactoring
        // got error or connection closed by client
        if (nbytes == 0) {
            // connection closed
            printf("Server Connection lost");
        } else {
            perror("recv");
        }
        close(fd); // bye!
        // FD_CLR(fd, master); // remove from master set
        // remove_user_fd(h, fd);
    } else {
        //Confirm Message - received it
        // if (send(fd, "100\n", 4, 0) == -1) {
        //     perror("send confirmation");
        // }
    }
    return nbytes;
} 

int get_message(int fd, char *buf) {
    int length = 0, nbytes;
    while(1) {
        nbytes = get_packet(fd, buf+length);
        if (nbytes == 0) {
            break;
        } else if ((buf[length + nbytes] = '\n')) {
            length = length + nbytes;///here mistake, before correction length was 0 in return
            break;
        } else {
            length += nbytes;
        }
    }
    return length;
}


