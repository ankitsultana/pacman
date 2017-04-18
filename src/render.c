#include "game_state.h"
#include <ncurses.h>
#include <locale.h>
#include <stdbool.h>
#include <assert.h>
#include <stdlib.h>
#include <string.h>

void render(game_state_t *gs_ptr) {
	//setlocale(LC_ALL, "");
	clear();
	int r, c, i;
	char temp[] = "x";
	int max_x, max_y;
	getmaxyx(stdscr,max_y,max_x);
	int init_y = (max_y-gs_ptr->num_rows)/2;
	int init_x = (max_x-(2*gs_ptr->num_cols))/2;
	// render grid
	for(r = 0; r < gs_ptr->num_rows; r++) {
		int curr_c;
		for(c = 0, curr_c = 0; c < gs_ptr->num_cols; c++, curr_c += 2) {
			temp[0] = gs_ptr->grid[r][c];
			if(temp[0] == 'X') {
				mvprintw(init_y+r, init_x+curr_c, "X");
				mvprintw(init_y+r, init_x+curr_c + 1, "X");
			} else {
				mvprintw(init_y+r, init_x+curr_c, temp);
				mvprintw(init_y+r, init_x+curr_c + 1, " ");
			}
		}
	}
	// render players
	for(i=0; i<gs_ptr->num_players; i++){
		char display_string[3];
		display_string[0] = gs_ptr->players[i]->username[0];
		display_string[1] = gs_ptr->players[i]->username[1];
		display_string[2] = '\0';
		r = gs_ptr->players[i]->pos.row;
		c = gs_ptr->players[i]->pos.col; 
		mvprintw(init_y+r,init_x+2*c,display_string);
	}
	refresh();
}

int render_main() {
	game_state_t* test = (game_state_t*)malloc(sizeof(game_state_t));
	test->num_rows = 16, test->num_cols = 16;
	test->grid = (char**)malloc(sizeof(char*) * 3);
	int i;
	for(i = 0; i < 3; i++) {
		test->grid[i] = (char*)malloc(sizeof(char) * 3);
		int j;
		for(j = 0; j < 3;j ++) {
			test->grid[i][j] = 'X';
		}
	}
	render(test);
	return 0;
}
