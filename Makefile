CC = gcc
STD = -std=c99 
WARN = -Wall -Werror -pedantic 
FLAGS = $(STD) $(WARN)

all: client server

hash_function: 
	$(CC) $(FLAGS) -o hash_function.o server/password/hash_function.c	

client: socket_client.o
	$(CC) $(FLAGS) -o client socket_client.o create_socket.c

server: socket_server.o
	$(CC) $(FLAGS) -o server socket_server.o create_socket.c

rebuild: clean all

clean:
	rm *.o server client
