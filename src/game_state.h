#ifndef GAME_STATE_H

#define GAME_STATE_H

#include <stdbool.h>
#include <stdio.h>

typedef struct game_state_t {
	int num_rows, num_cols;
	char** grid;
	bool corrupt;
} game_state_t;

game_state_t* parse_game_state_message(game_state_t* game_state, char * message);
void print_game_state(game_state_t*, FILE*);

#endif
