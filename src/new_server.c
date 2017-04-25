#include <sys/time.h>
#include <assert.h>
#include <time.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <curses.h>
#include <arpa/inet.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <netinet/in.h>
#include <unistd.h>
#include <pthread.h>

#include "util.h"
#include "networking.h"
#include "game_state.h"
#include "player.h"

#define PLAYERS_PER_GAME 2
#define TICK_TOCK 300

const char usage_fmt[] = "usage: %s server_port\n";
const char map_filename[] = "../maps/1.map";
char MAP[1000];
int st_r[] = {1, 14, 1, 14};
int st_c[] = {1, 1, 14, 14};

FILE * error_log;
const char* argv0;
int sockfd_listen;

player_list_t plist;

void cry_usage(){
	fprintf(error_log, usage_fmt, argv0);
	exit(2);
}

void* player_thread_func(void*);
void* game_thread_func(void*);

void create_map_str(char **grid) {
  int size = 0, i, j;
  sprintf(MAP, "%d %d\n", 16, 16);
  int iter = strlen(MAP);
  for(i = 0; i < 16; i++) {
    for(j = 0; j < 16; j++) {
      MAP[iter++] = grid[i][j];
    }
    MAP[iter++] = '\n';
  }
  MAP[iter++] = '\0';
}

void * listener_thread_func(void * arg) {
	// listen on listening socket wiht a maximum of 5 connections queued
	while(listen(sockfd_listen, 5) == 0) {
		// establish connection with the new player
		struct sockaddr_storage client_addr;
		socklen_t client_addr_sizeof = sizeof(client_addr);
		int sockfd = accept(sockfd_listen, (struct sockaddr *) &client_addr, &client_addr_sizeof);
		print_sock_info(sockfd, error_log);

		player_t* player = get_new_player(-1, NULL);
		player->sockfd = sockfd;
		player->ifp = fdopen(sockfd, "r");
		player->ofp = fdopen(sockfd, "w");
		setvbuf(player->ofp, NULL, _IONBF, BUFSIZ);
    bool ok = true;
    if(plist.size == MAX_PLAYERS) {
      fprintf(player->ofp, "reject\n\n");
      free(player);
      ok = false;
    } else {
      fscanf(player->ifp, "%s", player->username);
      int i;
      for(i = 0; i < MAX_PLAYERS; i++) {
        if(plist.arr[i] != NULL && strcmp(plist.arr[i]->username, player->username) == 0) { // TODO : Compare only first two chars
          fprintf(player->ofp, "taken\n\n");
          ok = false;
          free(player);
          break;
        }
      }
    }
    if(ok) {
      player->player_id = add_player_to_list(&plist, player);
      fprintf(player->ofp, "accept\n%d\n\n", player->player_id);
	  player->status = UNALLOCATED;
      plist.unallocated++;
      if(plist.unallocated % PLAYERS_PER_GAME == 0) {
        int i;
        game_state_t* game = get_new_game();
        FILE *map_file = fopen(map_filename, "r");
        fscanf(map_file, "%d %d\n", &game->num_rows, &game->num_cols);
        game->grid = (char**)malloc(sizeof(char*) * game->num_rows);
        for(i = 0; i < game->num_rows; i++) {
          game->grid[i] = (char*)malloc(sizeof(char) * (game->num_cols+3));
          fgets(game->grid[i], game->num_cols + 3, map_file);
          assert(game->grid[i][game->num_cols+1] == '\0');
        }
        create_map_str(game->grid);
        game->num_players = PLAYERS_PER_GAME;
        int ctr = 0;
        for(i = 0; i < MAX_PLAYERS; i++) {
          if(plist.arr[i] != NULL && plist.arr[i]->status == UNALLOCATED) {
            game->players[ctr++] = plist.arr[i];
          }
        }
        ctr = 0;
        pthread_t game_thread_id;
        pthread_create(&game_thread_id, NULL, game_thread_func, (void*)game);
        for(i = 0; i < MAX_PLAYERS; i++) {
          if(plist.arr[i] != NULL && plist.arr[i]->status == UNALLOCATED) {
            pthread_t player_thread_id;
            plist.arr[i]->status = PLAYING;
            plist.arr[i]->game = game;
            plist.arr[i]->pos.row = st_r[ctr];
            plist.arr[i]->pos.col = st_c[ctr];
            ctr++;
            pthread_create(&player_thread_id, NULL, player_thread_func, (void*)plist.arr[i]);
          }
        }
      }
    }
	}
	return NULL;
}

bool is_empty(char c) {
  return c == ' ' || c == '*' || c == '.';
}

void* game_thread_func(void *arg) {
  int i, j;
  game_state_t* game = arg;
  for(i = 0; i < game->num_players; i++) {
    game->grid[st_r[i]][st_r[j]] = 'P';
  }
  player_t* player;
  printf("Number of Players: %d\n", game->num_players);
  for(i = 0; i < game->num_players; i++) {
    player = (player_t*)game->players[i];
    printf("USER: %s has joined the game\n", player->username);
    fflush(stdout);
    fprintf(player->ofp, "init\n");
    FILE *map_file = fopen(map_filename, "r");
    char buf[200];
    while(fgets(buf, 200, map_file) != NULL) {
      fprintf(player->ofp, "%s", buf);
    }
    printf("SENT map to USER: %s\n", player->username);
    fflush(stdout);
    for(j = 0; j < game->num_players; j++) {
      fprintf(player->ofp, "%d %d %d %d %d %d %s\n",
              game->players[j]->player_id,
              game->players[j]->pos.row,
              game->players[j]->pos.col,
              game->players[j]->c_dir,
              game->players[j]->i_dir,
              game->players[j]->score,
              game->players[j]->username);
    }
    fprintf(player->ofp, "\n");
  }
  sleep(3);
  for(i = 0; i < game->num_players; i++) {
    game->players[i]->flag = 1;
  }
  char buf[300];
  while(true) {
    usleep(TICK_TOCK * 1000);
    for(i = 0; i < game->num_players; i++) {
      player = game->players[i];
      int r, c;
      r = player->pos.row, c = player->pos.col;
      switch(player->i_dir) {
        case UP:
          if(is_empty(game->grid[r-1][c])) {// == ' ' || game->grid[r-1][c] == '.') {
            player->c_dir = UP;
          }
          break;
        case DOWN:
          if(is_empty(game->grid[r+1][c])) {// == ' ' || game->grid[r+1][c] == '.') {
            player->c_dir = DOWN;
          }
          break;
        case LEFT:
          if(is_empty(game->grid[r][c-1])) {// == ' ' || game->grid[r][c-1] == '.') {
            player->c_dir = LEFT;
          }
          break;
        case RIGHT:
          if(is_empty(game->grid[r][c+1])) {// == ' ' || game->grid[r][c+1] == '.') {
            player->c_dir = RIGHT;
          }
          break;
        case NODIR:
          break;
        default:
          assert(false);
          break;
      }
      switch(player->c_dir) {
        case UP:
          if(is_empty(game->grid[r-1][c])) {// == ' ' || game->grid[r-1][c] == '.') {
            player->score += game->grid[r-1][c] == '.';
            game->grid[r][c] = ' ';
            game->grid[r-1][c] = 'P';
            player->pos.row = r - 1;
            player->pos.col = c;
          }
          break;
        case DOWN:
          if(is_empty(game->grid[r+1][c])) {// == ' ' || game->grid[r+1][c] == '.') {
            player->score += game->grid[r+1][c] == '.';
            game->grid[r][c] = ' ';
            game->grid[r+1][c] = 'P';
            player->pos.row = r + 1;
            player->pos.col = c;
          }
          break;
        case LEFT:
          if(is_empty(game->grid[r][c-1])) {// == ' ' || game->grid[r][c-1] == '.') {
            player->score += game->grid[r][c-1] == '.';
            game->grid[r][c] = ' ';
            game->grid[r][c-1] = 'P';
            player->pos.row = r;
            player->pos.col = c - 1;
          }
          break;
        case RIGHT:
          if(is_empty(game->grid[r][c+1])) {// == ' ' || game->grid[r][c+1] == '.') {
            player->score += game->grid[r][c+1] == '.';
            game->grid[r][c] = ' ';
            game->grid[r][c+1] = 'P';
            player->pos.row = r;
            player->pos.col = c + 1;
          }
          break;
        case NODIR:
          break;
        default:
          assert(false);
          break;
      }
    }
    int base = 0;
    for(i = 0; i < game->num_players; i++) {
      player = game->players[i];
      sprintf(buf + base, "%d %d %d %d %d %d %s\n",
          player->player_id,
          player->pos.row,
          player->pos.col,
          player->c_dir,
          player->i_dir,
          player->score,
          player->username);
      base = strlen(buf);
    }
    printf("Sending: %s\n", buf);
    create_map_str(game->grid);
    for(i = 0; i < game->num_players; i++) {
      fprintf(game->players[i]->ofp, "fullupd\n%s%s\n", MAP, buf);
    }
  }
  return NULL;
}

void* player_thread_func(void* arg) {
  int i;
  player_t* player = (player_t*)arg;
  game_state_t* game = (game_state_t*)player->game;
	char ch;
	while(true) {
		ch = fgetc(player->ifp);
    if(ch == EOF) {
      my_perror();
      break;
    }
    if(player->flag == 1) {
      switch(ch) {
        case 'w':
          player->i_dir = UP;
          break;
        case 'a':
          player->i_dir = LEFT;
          break;
        case 's':
          player->i_dir = DOWN;
          break;
        case 'd':
          player->i_dir = RIGHT;
          break;
      }
    } else {
      switch(ch) {
        case ' ':
        case '\n':
          break;
        case 'q':
          // quit
        default:
          fprintf(stderr, "%d (%s) pressed (%c)\n", player->player_id, player->username, ch);
      }
    }
	}
	return NULL;
}

int main(int argc, char ** argv) {
	// open error logfile
	error_log = stderr;

	// parse command line args
	argv0 = argv[0];
	if(argc != 2) {
		cry_usage();
	}
	int port_no = atoi(argv[1]);

	// open up a socket and bind port_no to it
	sockfd_listen = get_server_socket(port_no);

	// start listening for incoming requests
	listener_thread_func(NULL);
	return 0;
}
