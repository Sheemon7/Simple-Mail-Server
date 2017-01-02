CC = gcc
STD = -std=c99 
WARN = -Wall -Werror -pedantic 
FLAGS = $(STD) $(WARN)

all: server.o 
	$(CC) $(FLAGS) -o server server.o

rebuild: clean all

clean:
	rm *.o server
