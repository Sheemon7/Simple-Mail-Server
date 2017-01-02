CC = gcc
STD = -std=c99 
WARN = -Wall -Werror -pedantic 
FLAGS = $(STD) $(WARN)

all: server.o prepare_server.o 
	$(CC) $(FLAGS) -o server prepare_server.o server.o

rebuild: clean all

clean:
	rm *.o server
