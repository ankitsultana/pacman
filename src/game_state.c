#include "game_state.h"
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

#define MAX_MSG_LEN 1300


// Read map as a string and populate game_state struct

game_state_t* parse_game_state_message(game_state_t* game_state, char * message){
	if(game_state == NULL)
		game_state = (game_state_t*)malloc(sizeof(game_state_t));
	char * token = strtok(message,"\n");
	int i,j;
	if(token == NULL){
		//error
		game_state->corrupt = true;
		fprintf(stderr,"%s: Incomplete Message\n",__func__);
		return game_state;
	}
	sscanf(token,"%d%d",&game_state->num_rows,&game_state->num_cols);
	fprintf(stderr,"\"%s\"\n",token);
	game_state->grid = (char**)malloc(game_state->num_rows*sizeof(char*));
	for(i=0; i<game_state->num_rows; i++){
		token = strtok(NULL,"\n");
		fprintf(stderr,"\"%s\"\n",token);
		if(token == NULL){
			//error
			game_state->corrupt = true;
			fprintf(stderr,"%s: Incomplete Message\n",__func__);
			return game_state;
		}
		game_state->grid[i] = (char*)malloc(game_state->num_cols*sizeof(char));
		for(j=0; j<game_state->num_cols; j++) {
			if(token[j] == '\0'){
				game_state->corrupt = true;
				fprintf(stderr,"%s: Inconsistent row length\n",__func__);
				return game_state;
			}
			game_state->grid[i][j] = token[j];
		}
	}
	token = strtok(NULL,"\n");
	fprintf(stderr,"\"%s\"\n",token);
	if(token == NULL)
		return game_state;
	return game_state;
}

void print_game_state(game_state_t * gs, FILE * fs){
	fprintf(fs,"%d\n",gs->num_rows);
	fprintf(fs,"%d\n",gs->num_cols);
	int i;
	for(i=0; i<gs->num_rows; i++)
		fprintf(fs,"%s\n",gs->grid[i]);
}
