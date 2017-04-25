// This file can be used for both client as well as server
#ifndef H_PLAYER
#define H_PLAYER

#include <stdio.h>

typedef enum {
	UNREGISTERED,
	UNALLOCATED,
	PLAYING,
	EXITED
} player_status_t;

typedef enum {
	UP,
	DOWN,
	LEFT,
	RIGHT,
	NODIR
} dir_t;

typedef struct pos_t {
	int row;
	int col;
} pos_t;

typedef struct player_t {
	int player_id;
	pos_t pos;
	dir_t i_dir;
	dir_t c_dir;
	player_status_t status;
	int score;
	int flag;
	void * game;
	char username[30];
	int sockfd;
	FILE* ifp;
	FILE* ofp;
} player_t;

#define MAX_PLAYERS 200

typedef struct player_list_t {
	player_t* arr[MAX_PLAYERS];
	int size;
	int unallocated;
} player_list_t;

player_t * get_new_player(int, char*);
player_t* get_player_by_username(player_list_t* pplist, const char* username);
int add_player_to_list(player_list_t* pplist, player_t* pl);
void remove_player_from_list(player_list_t* pplist, int id);

#endif
