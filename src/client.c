#define _POSIX_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <unistd.h>

#include <ncurses.h>
#include <pthread.h>

#include "game_state.h"
#include "player.h"
#include "networking.h"
#include "render.h"



const char usage_fmt[] = "usage: %s server_ip server_port username\n";

const char app_name[] = "pacmen";
const char* argv0;
FILE * error_log;

void cry_usage() {
	fprintf(error_log, usage_fmt, argv0);
	exit(2);
}

#define BUFSIZE 2048
#define HEADER_SZ 10

player_t * me;

pthread_mutex_t me_mutex;

void free_me(){
	fprintf(error_log,"Freeing me\n");
	free_game(me->game);
	me = NULL;
}

char* username;
int sockfd;
char* server_ipaddr;
int server_port;
player_status_t player_status = UNREGISTERED;
FILE* sockifp;
FILE* sockofp;

char buffer[BUFSIZE];

void send_username(const char* username) {
	fprintf(sockofp, "%s\n", username);
}

void send_char(char ch) {
	fprintf(sockofp, "%c", ch);
}

void* sender_thread_func(void* arg) {
	send_username(username);
	while(true) {
		char ch = getch();
		send_char(ch);
	}
	return NULL;
}

void init_curses(){
	initscr();
	cbreak();
	noecho();
	curs_set(0);
}

void display_message_center(const char * display_message){
		clear();
		int display_message_len = strlen(display_message);
		int max_y, max_x;
		getmaxyx(stdscr,max_y,max_x);
		mvprintw(max_y/2,(max_x-display_message_len)/2,display_message);
		refresh();
}

#define unexpected_message_exception(message,player_status) fprintf(error_log,"Unexpected %s Message to Player with Status %d\n", message, player_status);

int execute_message(char* message) {
	pthread_mutex_lock(&me_mutex);
	fprintf(error_log, "Message:\n%s", buffer);
	
	char message_header[HEADER_SZ];
	char possible_headers[][HEADER_SZ] = 
	{
#define X(a) #a,
#include "server_message_headers.xmac"
#undef X
	};
	typedef enum server_message_header_t {
#define X(a) SMH_##a,
#include "server_message_headers.xmac"
#undef X
		num_possible_headers
	} server_message_header_t;
	sscanf(message,"%s",message_header);
	int message_body_offset = strlen(message_header);
	int i;
	server_message_header_t ans = num_possible_headers;
	for(i=0; i<(int)num_possible_headers; i++)
		if(!strcmp(message_header,possible_headers[i])) {
			ans = i;
			break;
		}
	if(ans == num_possible_headers) {
		fprintf(error_log,"Invalid Header %s\n",message_header);
		return 1;
	}
	switch(ans){
		case SMH_accept:;
			display_message_center("Waiting to be allocated ... ");
			if(player_status == UNREGISTERED){
				player_status = UNALLOCATED;
				int player_id = -1, row, col;
				sscanf(message+message_body_offset,"%d%d%d",&player_id, &row, &col);
				me = get_new_player(player_id,username);
				me->pos.row = row;
				me->pos.col = col;
				me->c_dir = RIGHT;
				me->i_dir = RIGHT;
			} else {
				unexpected_message_exception(possible_headers[ans],player_status);
			}
			break;

		case SMH_reject:
			if(player_status == UNREGISTERED){
				player_status = EXITED;
				free_me();
			} else {
				unexpected_message_exception(possible_headers[ans],player_status);
			}
			break;

		case SMH_taken:
			if(player_status == UNREGISTERED) {
				player_status = EXITED;
				free_me();
			} else {
				unexpected_message_exception(possible_headers[ans],player_status);
			}
			break;

		case SMH_init:;
			if(player_status == UNALLOCATED) {
				player_status = PLAYING;
				me->game = get_new_game();
				add_player(me->game,me);
				parse_game_state_message(me->game,message+message_body_offset,error_log);
				render(me->game);
			} else {
				unexpected_message_exception(possible_headers[ans],player_status);
			}
			break;

		case SMH_fullupd:
			if(player_status == PLAYING) {
				player_status = PLAYING;
				parse_game_state_message(me->game,message+message_body_offset,error_log);
				render(me->game);
			} else {
				unexpected_message_exception(possible_headers[ans],player_status);
			}
			break;

		case SMH_partupd:
			if(player_status == PLAYING)
				player_status = PLAYING;
			else
				unexpected_message_exception(possible_headers[ans],player_status);
			break;

		case SMH_end:
			if(player_status == PLAYING)
				player_status = UNALLOCATED;
			else
				unexpected_message_exception(possible_headers[ans],player_status);
			break;
	
		case SMH_quit:
			player_status = EXITED;
			free_me();
			return -1;
			break;
		default:
			break;
	}
	pthread_mutex_unlock(&me_mutex);
	return 0;
}

#undef unexpected_message_exception

void* receiver_thread_func(void* arg) {
	int offset = 0;
	while(fgets(buffer+offset, BUFSIZE, sockifp) != NULL) {
		bool empty_line = (buffer[offset] == '\n');
		while(buffer[offset] != '\0') {
			offset++;
		}
		if(empty_line) {
			offset = 0;
			int status = execute_message(buffer);
			if(status == -1){
				fprintf(error_log, "Quitting Game Now...\n");
				return NULL;
			}
		}
	}
	fprintf(error_log, "Error reading data from server.\n");
	return NULL;
}

pos_t above(pos_t ref){
	pos_t ret = ref;
	ret.row -= 1;
	return ret;
}

pos_t below(pos_t ref){
	pos_t ret = ref;
	ret.row += 1;
	return ret;
}

pos_t leftof(pos_t ref){
	pos_t ret = ref;
	ret.col -= 1;
	return ret;
}

pos_t rightof(pos_t ref){
	pos_t ret = ref;
	ret.col += 1;
	return ret;
}

bool valid(pos_t pos){
	if(((game_state_t*)(me->game))->grid[pos.row][pos.col] != 'X') return true;
	else return false;
	return true;
}

void update_player_dir(int player_index){
	if(((game_state_t*)(me->game))->players[player_index]->c_dir == ((game_state_t*)(me->game))->players[player_index]->i_dir) return;
	else {
		pos_t current_pos = ((game_state_t*)(me->game))->players[player_index]->pos;
		switch(((game_state_t*)(me->game))->players[player_index]->i_dir){
			case UP:
				if(valid(above(current_pos)))
					((game_state_t*)(me->game))->players[player_index]->c_dir = ((game_state_t*)(me->game))->players[player_index]->i_dir;
				break;
			case DOWN:
				if(valid(below(current_pos)))
					((game_state_t*)(me->game))->players[player_index]->c_dir = ((game_state_t*)(me->game))->players[player_index]->i_dir;
				break;
			case LEFT:
				if(valid(leftof(current_pos)))
					((game_state_t*)(me->game))->players[player_index]->c_dir = ((game_state_t*)(me->game))->players[player_index]->i_dir;
				break;
			case RIGHT:
				if(valid(rightof(current_pos)))
					((game_state_t*)(me->game))->players[player_index]->c_dir = ((game_state_t*)(me->game))->players[player_index]->i_dir;
				break;
			case NODIR:
				break;
			default:
				fprintf(error_log,"%s: Invalid i_dir for player having id %d\n",__func__,((game_state_t*)(me->game))->players[player_index]->player_id);
				break;
		}
	}
}

void update_player_pos(int player_index){
	switch(((game_state_t*)(me->game))->players[player_index]->c_dir){
		case UP:
			if(valid(above(((game_state_t*)(me->game))->players[player_index]->pos)))
				((game_state_t*)(me->game))->players[player_index]->pos = above(((game_state_t*)(me->game))->players[player_index]->pos);
			break;
		case DOWN:
			if(valid(below(((game_state_t*)(me->game))->players[player_index]->pos)))
				((game_state_t*)(me->game))->players[player_index]->pos = below(((game_state_t*)(me->game))->players[player_index]->pos);
			break;
		case LEFT:
			if(valid(leftof(((game_state_t*)(me->game))->players[player_index]->pos)))
				((game_state_t*)(me->game))->players[player_index]->pos = leftof(((game_state_t*)(me->game))->players[player_index]->pos);
			break;
		case RIGHT:
			if(valid(rightof(((game_state_t*)(me->game))->players[player_index]->pos)))
				((game_state_t*)(me->game))->players[player_index]->pos = rightof(((game_state_t*)(me->game))->players[player_index]->pos);
			break;
		case NODIR:
			break;
	}
	fprintf(error_log, "New position (%s): row = %d col = %d\n", ((game_state_t*)(me->game))->players[player_index]->username,((game_state_t*)(me->game))->players[player_index]->pos.row,((game_state_t*)(me->game))->players[player_index]->pos.col);
}

void update(){
	if(player_status == PLAYING){
		fprintf(error_log, "Updater thread active!!\n");
		pthread_mutex_lock(&me_mutex);
		fprintf(error_log, "Lock acquired!\n");
		// Perform updates
		int i;
		// Update directions
		for(i=0; i<((game_state_t*)(me->game))->num_players; i++)
			update_player_dir(i);
		// Update positions
		for(i=0; i<((game_state_t*)(me->game))->num_players; i++)
			update_player_pos(i);
		// TODO: Update score

		// Render screen
		render(me->game);
		pthread_mutex_unlock(&me_mutex);
		sleep(1);
	}
}

void* updater_thread_func(void* arg) {
	while(true){
		update();
	}
}

int main(int argc, char* argv[]) {
	// open error logfile
	error_log = fopen("../secret/error.log","w");

	// parse cmdline params
	argv0 = argv[0];
	if(argc != 4) {
		cry_usage();
	}
	server_ipaddr = argv[1];
	server_port = atoi(argv[2]);
	username = argv[3];
	
	// connect to server
	sockfd = get_client_socket(server_ipaddr, server_port);
	sockifp = fdopen(sockfd, "r");
	sockofp = fdopen(sockfd, "w");
	setvbuf(sockofp, NULL, _IONBF, BUFSIZ);
	print_sock_info(sockfd, error_log);

	// init curses mode
	init_curses();

	// init me_mutex
	pthread_mutex_init(&me_mutex,NULL);
	
	// display initial splash screen
	display_message_center("Welcome to Multiplayer Pacman!");

	// create threads
	pthread_t sender_thread_id;
	pthread_create(&sender_thread_id, NULL, sender_thread_func, NULL);

	pthread_t updater_thread_id;
	pthread_create(&updater_thread_id, NULL, updater_thread_func, NULL);
	
	receiver_thread_func(NULL);
	fclose(sockifp);
	fclose(sockofp);
	fclose(error_log);
	pthread_mutex_destroy(&me_mutex);
	endwin();

	return 0;
}
