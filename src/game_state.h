// This file can be used by both client as well as server
#ifndef H_GAME_STATE

#define H_GAME_STATE

#include <stdbool.h>
#include <stdio.h>
#include "player.h"

typedef struct game_state_t {
	int num_rows, num_cols;
	char** grid;
	bool corrupt;
	int num_players;
	player_t * players[5];
} game_state_t;

void parse_game_state_message(game_state_t* game_state, char * message, FILE *);
void print_game_state(game_state_t*, FILE*);
game_state_t * get_new_game();
void free_game(game_state_t *);
void add_player(game_state_t *, player_t *);

#endif
