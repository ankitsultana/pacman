// This file can be used for both client as well as server
#ifndef H_PLAYER
#define H_PLAYER

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
	int score;
	void * game;
} player_t;

player_t * get_new_player(int);

#endif
