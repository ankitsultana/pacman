CC = gcc
CC_FLAGS = -g -lncurses -Wall

all: server client
server: server.c
	$(CC) $(CC_FLAGS) server.c -o server
client: client.c
	$(CC) $(CC_FLAGS) client.c -o client
