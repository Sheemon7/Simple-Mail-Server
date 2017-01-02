CC = gcc
STD = -std=c99 
WARN = -Wall -Werror -pedantic 
FLAGS = $(STD) $(WARN)

all: socket_client.o create_socket.o 
	$(CC) $(FLAGS) -o client socket_client.o create_socket.o

hash_function: 
	$(CC) $(FLAGS) -o hash_function.o server/password/hash_function.c	

rebuild: clean all

clean:
	rm *.o client
