#include "game_state.h"
#include <ncurses.h>
#include <locale.h>
#include <stdbool.h>
#include <assert.h>
#include <stdlib.h>
#include <string.h>

WINDOW * _game_window, * _message_window, * _scoreboard_window;
int SCOREBOARD_WIDTH = 20;

void init_curses(FILE * error_log) {
	initscr();
	cbreak();
	noecho();
	curs_set(0);
	_game_window = NULL;
	_message_window = NULL;
	_scoreboard_window = NULL;
	int max_x, max_y;
	getmaxyx(stdscr,max_y,max_x);
	if (max_x <= 20 || max_y <= 20) {
		endwin();
		fprintf(error_log,"Screen is too small!\n");
		exit(0);
	}
	SCOREBOARD_WIDTH = 21;
}

void init_message_window(FILE * error_log) {
	int max_y, max_x;
	getmaxyx(stdscr, max_y, max_x);
	_message_window = newwin(1, max_x, max_y-1, 0);
	wrefresh(_message_window);
}

void init_game_window(FILE * error_log) {
	int max_y, max_x;
	getmaxyx(stdscr, max_y, max_x);
	_game_window = newwin(max_y-1, max_x-SCOREBOARD_WIDTH, 0, 0);
	// _game_window = newwin(2, 30, 0, 0);
	// mvwprintw(_game_window, 0, 0, "GAME WINDOW !!");
	wrefresh(_game_window);
}

void init_scoreboard_window(FILE * error_log) {
	int max_y, max_x;
	getmaxyx(stdscr, max_y, max_x);
	_scoreboard_window = newwin(max_y-1, SCOREBOARD_WIDTH, 0, max_x-SCOREBOARD_WIDTH);
	// mvwprintw(_scoreboard_window, 0, 0, "SCOREBOARD WINDOW !!");
	wrefresh(_scoreboard_window);
}

void print_message(const char * msg, FILE * error_log) {
	if(_message_window == NULL) {
		fprintf(error_log, "%s: _message_window not inited\n", __func__);
		return;
	}
	mvwprintw(_message_window, 0, 0, msg);
	wrefresh(_message_window);
}

void destroy_message_window(FILE * error_log) {
	clear_window(_message_window, error_log);
	delwin(_message_window);
	_message_window = NULL;
}

void destroy_game_window(FILE * error_log) {
	clear_window(_game_window, error_log);
	delwin(_game_window);
	_game_window = NULL;
}

void destroy_scoreboard_window(FILE * error_log) {
	clear_window(_scoreboard_window, error_log);
	delwin(_scoreboard_window);
	_scoreboard_window = NULL;
}

void render_scoreboard(game_state_t * game, FILE* error_log) {
	if(_scoreboard_window == NULL) {
		fprintf(error_log, "%s: _scoreboard_window not inited\n", __func__);
		return ;
	}
	int i, curr_r = 0;
	int max_y, max_x;
	char title[] = "SCOREBOARD";
	int title_len = strlen(title);
	getmaxyx(_scoreboard_window, max_y, max_x);
	mvwprintw(_scoreboard_window, 0, (max_x-title_len)/2,"SCOREBOARD");
	for(i=0; i<game->num_players; i++) {
		char print_str[SCOREBOARD_WIDTH];
		sprintf(print_str, "%s\t%d", game->players[i]->username, game->players[i]->score);
		mvwprintw(_scoreboard_window, i+1, 0, print_str);
	}
	wrefresh(_scoreboard_window);
}

void render_game_screen(game_state_t *gs_ptr, FILE* error_log) {
	//setlocale(LC_ALL, "");
	if(_game_window == NULL) {
		fprintf(error_log, "%s: _game_window not inited\n", __func__);
		return ;
	}
	clear();
	int r, c, i;
	char temp[] = "x";
	int max_x, max_y;
	getmaxyx(_game_window,max_y,max_x);
	int init_y = (max_y-gs_ptr->num_rows)/2;
	int init_x = (max_x-(2*gs_ptr->num_cols))/2;
	// render grid
	for(r = 0; r < gs_ptr->num_rows; r++) {
		int curr_c;
		for(c = 0, curr_c = 0; c < gs_ptr->num_cols; c++, curr_c += 2) {
			temp[0] = gs_ptr->grid[r][c];
			if(temp[0] == 'X') {
				mvwprintw(_game_window, init_y+r, init_x+curr_c, "X");
				mvwprintw(_game_window, init_y+r, init_x+curr_c + 1, "X");
			} else {
				mvwprintw(_game_window, init_y+r, init_x+curr_c, temp);
				mvwprintw(_game_window, init_y+r, init_x+curr_c + 1, " ");
			}
		}
	}
	// render players
	for(i=0; i<gs_ptr->num_players; i++) {
		char display_string[3];
		display_string[0] = gs_ptr->players[i]->username[0];
		display_string[1] = gs_ptr->players[i]->username[1];
		display_string[2] = '\0';
		r = gs_ptr->players[i]->pos.row;
		c = gs_ptr->players[i]->pos.col; 
		mvwprintw(_game_window, init_y+r,init_x+2*c,display_string);
	}
	// render quit message
	print_message("Press Q to quit game", error_log);

	// render scoreborad
	render_scoreboard(gs_ptr, error_log);
	wrefresh(_game_window);
}

void render_welcome_screen(FILE * error_log) {
	clear();
	int max_x, max_y;
	getmaxyx(stdscr, max_y, max_x);
	char welcome_message[] = "Welcome to Multiplayer PacMan";
	char wait_message[] = "Please wait while we register you";
	int welcome_message_len = strlen(welcome_message);
	int wait_message_len = strlen(wait_message);
	mvprintw((max_y/2)-1, (max_x-welcome_message_len)/2, welcome_message);
	mvprintw((max_y/2)+1, (max_x-wait_message_len)/2, wait_message);
	refresh();
}

void render_unallocated_screen(FILE * error_log) {
	clear();
	int max_x, max_y;
	getmaxyx(stdscr, max_y, max_x);
	char wait_message[] = "Please wait while we find you a game...";
	int wait_message_len = strlen(wait_message);
	mvprintw(max_y/2, (max_x-wait_message_len)/2, wait_message);
	refresh();
}

void clear_window(WINDOW * win, FILE * error_log) {
	if(win == NULL) {
		fprintf(error_log, "%s: Window doesn't exist!\n", __func__);
		return ;
	}
	int max_y, max_x, i;
	getmaxyx(win, max_y, max_x);
	char spaces[max_x+1];
	for(i=0; i<max_x; i++)
		spaces[i] = ' ';
	spaces[i] = '\0';
	for(i=0; i<max_y; i++)
		mvwprintw(win, i, 0, spaces);
	wrefresh(win);
	return;
}

/*
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
*/
