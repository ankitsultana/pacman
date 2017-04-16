#include "player.h"
#include <stdlib.h>

player_t* get_new_player(int player_id){
	player_t * new_player = (player_t*)malloc(sizeof(player_t));
	new_player->player_id = player_id;
	new_player->pos.row = -1;
	new_player->pos.col = -1;
	new_player->c_dir = NODIR;
	new_player->i_dir = NODIR;
	new_player->score = 0;
	new_player->game = NULL;
	return new_player;
}
