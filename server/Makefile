CC = gcc
STD = -std=c99 
WARN = -Wall -Werror -pedantic 
FLAGS = $(STD) $(WARN)

all: server.o create_socket.o
	$(CC) $(FLAGS) -o server server.o create_socket.o

rebuild: clean all

clean:
	rm *.o server
