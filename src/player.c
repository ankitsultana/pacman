#include "player.h"
#include <stdlib.h>
#include <string.h>

player_t* get_new_player(int player_id, char * username){
	player_t * new_player = (player_t*)malloc(sizeof(player_t));
	new_player->player_id = player_id;
	new_player->pos.row = -1;
	new_player->pos.col = -1;
	new_player->c_dir = NODIR;
	new_player->i_dir = NODIR;
	new_player->status = UNREGISTERED;
	new_player->score = 0;
	new_player->game = NULL;
	new_player->ifp = NULL;
	new_player->ofp = NULL;
	new_player->sockfd = -1;
	if(username == NULL) {
		new_player->username[0] = '\0';
	}
	else {
		strcpy(new_player->username,username);
	}
	return new_player;
}

player_t* get_player_by_username(player_list_t* pplist, const char* username) {
	for(int i=0; i<MAX_PLAYERS; ++i) {
		if(pplist->arr[i] != NULL && strcmp(pplist->arr[i]->username, username) == 0) {
			return pplist->arr[i];
		}
	}
	return NULL;
}

int add_player_to_list(player_list_t* pplist, player_t* pl) {
	for(int i=0; i<MAX_PLAYERS; ++i) {
		if(pplist->arr[i] == NULL) {
			pplist->arr[i] = pl;
			return i;
		}
	}
	return -1;
}

void remove_player_from_list(player_list_t* pplist, int id) {
	pplist->arr[id] = NULL;
}
