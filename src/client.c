#define _POSIX_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>

#include <ncurses.h>
#include <pthread.h>

#include "networking.h"
#include "game_state.h"
#include "render.h"



const char usage_fmt[] = "usage: %s server_ip server_port username\n";

const char app_name[] = "pacmen";
const char* argv0;

void cry_usage() {
	fprintf(stderr, usage_fmt, argv0);
	exit(2);
}

#define BUFSIZE 2048
#define HEADER_SZ 10


char* username;
int sockfd;
char* server_ipaddr;
int server_port;
int player_status;
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


void execute_message(char* message) {
	fprintf(stderr, "Message:\n%s", buffer);
	
	char message_header[HEADER_SZ];
	int num_possible = 8;
	char possible_headers[][HEADER_SZ] = {"accept", "reject", "taken", "init", "fullupd", "partupd", "end", "quit"};
	sscanf(message,"%s",message_header);
	int message_body_offset = strlen(message_header);
	int i, ans = -1;
	for(i=0; i<num_possible; i++)
		if(!strcmp(message_header,possible_headers[i])) {
			ans = i;
			break;
		}
	if(ans == -1) {
		fprintf(stderr,"Invalid Header %s\n",message_header);
		return ;
	}
	switch(ans){
		case 0:
			mvprintw(0,0,"Waiting to be allocated...");
			refresh();
			break;

		case 3:;

			game_state_t * new_game = NULL;
			new_game = parse_game_state_message(new_game,message+message_body_offset);
			render(new_game);
			//print_game_state(new_game,stderr);
			break;
	}
}

void* receiver_thread_func(void* arg) {
	int offset = 0;
	while(fgets(buffer+offset, BUFSIZE, sockifp) != NULL) {
		bool empty_line = (buffer[offset] == '\n');
		while(buffer[offset] != '\0') {
			offset++;
		}
		if(empty_line) {
			offset = 0;
			execute_message(buffer);
		}
	}
	fprintf(stderr, "Error reading data from server.\n");
	return NULL;
}

int main(int argc, char* argv[]) {
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
	print_sock_info(sockfd, stderr);

	//init curses mode
	init_curses();
	
	// create threads
	pthread_t sender_thread_id;
	pthread_create(&sender_thread_id, NULL, sender_thread_func, NULL);
	
	receiver_thread_func(NULL);
	endwin();

	return 0;
}
