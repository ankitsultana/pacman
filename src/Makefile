CC = gcc
CC_FLAGS = -g -Wall
LINKER_FLAGS = -lncurses -lpthread

client: *.c *.h
	$(CC) $(CC_FLAGS) client.c networking.c util.c $(LINKER_FLAGS) -o client
