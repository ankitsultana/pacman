#include "render.h"
#include <unistd.h>

int main(){
	init_curses(stdout);
	init_message_window(stdout);
	print_message("This is some message",stdout);
	int max_y, max_x, i;
	init_game_window(stdout);
	getmaxyx(_game_window,max_y,max_x);
	for(i=1; i<max_y; i++) {
		mvwprintw(_game_window, i, 0, "This is the game window");
		wrefresh(_game_window);
	}
	sleep(3);
	destroy_message_window(stdout);
	sleep(3);
	destroy_game_window(stdout);
	init_scoreboard_window(stdout);
	getmaxyx(_scoreboard_window,max_y,max_x);
	for(i=1; i<max_y; i++){
		mvwprintw(_scoreboard_window, i, 0, "This is the scoreboard");
		wrefresh(_scoreboard_window);
	}
	sleep(3);
	destroy_scoreboard_window(stdout);
	sleep(3);
	endwin();
	return 0;
}
