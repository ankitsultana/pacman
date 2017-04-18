// This file can be used for both client as well as server
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include "game_state.h"
#include "player.h"

#define MAX_MSG_LEN 1300

game_state_t * get_new_game() {
	int i;
	game_state_t * ret = (game_state_t*)malloc(sizeof(game_state_t));
	ret->grid = NULL;
	ret->corrupt = false;
	ret->num_players = 0;
	for(i=0; i<5; i++) ret->players[i] = NULL;
	return ret;
}

void free_game(game_state_t * game){
	int i;
	for(i=0; i<game->num_rows; i++)
		free(game->grid[i]);
	free(game->grid);
	for(i=0; i<game->num_players; i++)
		free(game->players[i]);
	free(game);
}

void add_player(game_state_t * game, player_t * player){
	game->players[game->num_players++] = player;
	player->game = game;
}

// Read map as a string and populate game_state struct

void parse_game_state_message(game_state_t* game_state, char * message, FILE * error_log){
	char * token = strtok(message,"\n");
	int i,j;
	if(token == NULL){
		//error
		game_state->corrupt = true;
		fprintf(error_log,"%s: Incomplete Message\n",__func__);
		return;
	}
	sscanf(token,"%d%d",&game_state->num_rows,&game_state->num_cols);
	//fprintf(error_log,"\"%s\"\n",token);
	if(game_state->grid == NULL){
		game_state->grid = (char**)malloc(game_state->num_rows*sizeof(char*));
		for(i=0; i<game_state->num_rows; i++)
			game_state->grid[i] = NULL;
	}
	for(i=0; i<game_state->num_rows; i++){
		token = strtok(NULL,"\n");
		//fprintf(error_log,"\"%s\"\n",token);
		if(token == NULL){
			//error
			game_state->corrupt = true;
			fprintf(error_log,"%s: Incomplete Message\n",__func__);
			return ;
		}
		if(game_state->grid[i] == NULL){
			game_state->grid[i] = (char*)malloc(game_state->num_cols*sizeof(char));
		}
		for(j=0; j<game_state->num_cols; j++) {
			if(token[j] == '\0'){
				game_state->corrupt = true;
				fprintf(error_log,"%s: Inconsistent row length\n",__func__);
				return ;
			}
			game_state->grid[i][j] = token[j];
		}
	}
	token = strtok(NULL,"\n");
	//fprintf(error_log,"\"%s\"\n",token);
	if(token == NULL)
		return ;
	//return game_state;
	while(token != NULL){
		print_game_state(game_state, error_log);
		int player_id, row, col, i_dir, c_dir, score;
		char username[30];
		sscanf(token,"%d%d%d%d%d%d%s",&player_id,&row,&col,&c_dir,&i_dir,&score,username);
		token = strtok(NULL,"\n");
		//fprintf(error_log,"\"%s\"\n",token);
		bool player_exists = false;
		for(i=0; i<5; i++){
			if(game_state->players[i] != NULL && player_id == game_state->players[i]->player_id){
				player_exists = true;
				break;
			} else if(game_state->players[i] == NULL) {
				break;
			}
		}

		if(player_exists){
			//game_state->grid[game_state->players[i]->pos.row][game_state->players[i]->pos.col] = ' ';
			game_state->players[i]->pos.row = row;
			game_state->players[i]->pos.col = col;
			game_state->players[i]->i_dir = i_dir;
			game_state->players[i]->c_dir = c_dir;
			game_state->players[i]->score = score;
			//game_state->grid[game_state->players[i]->pos.row][game_state->players[i]->pos.col] = (char)(i+'0');
		}else{
			if(i == 5){
				// Game is already full. More players cannot be accomodated
				fprintf(error_log,"%s: Game Full!\n", __func__);
			}else{
				player_t * new_player = get_new_player(player_id,username);
				fprintf(error_log, "%s: New Player: %s\n", __func__ , username);
				new_player->pos.row = row;
				new_player->pos.col = col;
				new_player->i_dir = i_dir;
				new_player->c_dir = c_dir;
				new_player->score = score;
				add_player(game_state,new_player);
				//game_state->grid[game_state->players[i]->pos.row][game_state->players[i]->pos.col] = (char)(game_state->num_players-1+(int)('0'));
			}
		}
	}
}

void print_game_state(game_state_t * gs, FILE * fs){
	fprintf(fs,"==== %s ====\n",__func__);
	fprintf(fs,"%d\n",gs->num_rows);
	fprintf(fs,"%d\n",gs->num_cols);
	int i;
	fprintf(fs, "GRID: \n");
	for(i=0; i<gs->num_rows; i++)
		fprintf(fs,"%s\n",gs->grid[i]);
	fprintf(fs, "PLAYERS\n");
	for(i=0; i<gs->num_players; i++){
		fprintf(fs, "%s\t\t%d\n", gs->players[i]->username, gs->players[i]->player_id);
	}
	fprintf(fs,"===========\n");
}
