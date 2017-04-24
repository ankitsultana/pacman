#ifndef H_RENDER
#define H_RENDER

#include "game_state.h"
#include <ncurses.h>

extern WINDOW * _game_window, * _message_window, * _scoreboard_window;

void init_curses(FILE *);
void init_message_window(FILE *);
void init_game_window(FILE *);
void init_scoreboard_window(FILE *);
void print_message(const char *, FILE*);
void clear_window(WINDOW *, FILE*);
void destroy_message_window(FILE*);
void destroy_game_window(FILE *);
void destroy_scoreboard_window(FILE *);
void render_scoreboard(game_state_t*, FILE*);
void render_game_screen(game_state_t *, FILE*);
void render_welcome_screen(FILE *);
void render_unallocated_screen(FILE *); 

#endif
