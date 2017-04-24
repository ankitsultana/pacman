CC = gcc
CC_FLAGS = -g -Wall
LINKER_FLAGS = -lncurses -lpthread
sources = player.c networking.c util.c game_state.c

client: *.c *.h
	$(CC) $(CC_FLAGS) $(sources) client.c render.c $(LINKER_FLAGS) -o client
server: *.c *.h
	$(CC) $(CC_FLAGS) $(sources) new_server.c $(LINKER_FLAGS) -o server
