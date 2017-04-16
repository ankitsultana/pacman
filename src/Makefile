CC = gcc
CC_FLAGS = -g -Wall
LINKER_FLAGS = -lncurses -lpthread

client: *.c *.h
	$(CC) $(CC_FLAGS) player.c client.c networking.c util.c game_state.c render.c $(LINKER_FLAGS) -o client
