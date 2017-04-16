// This file can be used for both client as well as server
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include "game_state.h"

#define MAX_MSG_LEN 1300

game_state_t * get_new_game() {
	int i;
	game_state_t * ret = (game_state_t*)malloc(sizeof(game_state_t));
	ret->grid = NULL;
	ret->corrupt = false;
	ret->num_players = 0;
	for(i=0; i<10; i++) ret->players[i] = NULL;
	return ret;
}

void free_game(game_state_t * game){
	int i;
	for(i=0; i<game->num_rows; i++)
		free(game->grid[i]);
	for(i=0; i<game->num_players; i++)
		free(game->players[i]);
}

void add_player(game_state_t * game, player_t * player){
	game->players[game->num_players++] = player;
	player->game = game;
}

// Read map as a string and populate game_state struct

void parse_game_state_message(game_state_t* game_state, char * message){
	char * token = strtok(message,"\n");
	int i,j;
	if(token == NULL){
		//error
		game_state->corrupt = true;
		fprintf(stderr,"%s: Incomplete Message\n",__func__);
		return;
	}
	sscanf(token,"%d%d",&game_state->num_rows,&game_state->num_cols);
	fprintf(stderr,"\"%s\"\n",token);
	if(game_state->grid == NULL){
		game_state->grid = (char**)malloc(game_state->num_rows*sizeof(char*));
		for(i=0; i<game_state->num_rows; i++)
			game_state->grid[i] = NULL;
	}
	for(i=0; i<game_state->num_rows; i++){
		token = strtok(NULL,"\n");
		fprintf(stderr,"\"%s\"\n",token);
		if(token == NULL){
			//error
			game_state->corrupt = true;
			fprintf(stderr,"%s: Incomplete Message\n",__func__);
			return ;
		}
		if(game_state->grid[i] == NULL){
			printf("%lu %lu\n", game_state->num_cols, game_state);
			game_state->grid[i] = (char*)malloc(game_state->num_cols*sizeof(char));
		}
		for(j=0; j<game_state->num_cols; j++) {
			if(token[j] == '\0'){
				game_state->corrupt = true;
				fprintf(stderr,"%s: Inconsistent row length\n",__func__);
				return ;
			}
			game_state->grid[i][j] = token[j];
		}
	}
	token = strtok(NULL,"\n");
	fprintf(stderr,"\"%s\"\n",token);
	if(token == NULL)
		return ;
	//return game_state;
	while(token != NULL){
		int player_id, row, col, i_dir, c_dir;
		char username[100];
		sscanf(token,"%d%d%d%d%d%s",&player_id,&row,&col,&c_dir,&i_dir,username);

	}
}

void print_game_state(game_state_t * gs, FILE * fs){
	fprintf(fs,"%d\n",gs->num_rows);
	fprintf(fs,"%d\n",gs->num_cols);
	int i;
	for(i=0; i<gs->num_rows; i++)
		fprintf(fs,"%s\n",gs->grid[i]);
}