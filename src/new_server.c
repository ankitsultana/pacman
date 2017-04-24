#include <sys/time.h>
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

#define PLAYERS_PER_GAME 4

const char usage_fmt[] = "usage: %s server_port\n";

FILE * error_log;
const char* argv0;
int sockfd_listen;

player_list_t plist;

void cry_usage(){
	fprintf(error_log, usage_fmt, argv0);
	exit(2);
}

void* player_thread_func(void*);

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
		player_thread_func(player);
	}
	return NULL;
}

void* player_thread_func(void* arg) {
	int i;
	player_t* player = (player_t*)arg;

	if(plist.size == MAX_PLAYERS) {
		fprintf(player->ofp, "reject\n\n");
		free(player);
		return NULL;
	}

	// get the username

	fscanf(player->ifp, "%s", player->username);
	// TODO: change to fgets to improve security

	for(i=0; i<MAX_PLAYERS; ++i) {
		if(plist.arr[i] != NULL && strcmp(plist.arr[i]->username, player->username) == 0) {
			fprintf(player->ofp, "taken\n\n");
			free(player);
			return NULL;
		}
	}

	player->player_id = add_player_to_list(&plist, player);
	fprintf(player->ofp, "accept\n%d\n\n", player->player_id);
	player->status = UNALLOCATED;
	(plist.unallocated)++;

	if(plist.unallocated % PLAYERS_PER_GAME == 0) {
		// allocate a game
	}

	char ch;
	while(true) {
		ch = fgetc(player->ifp);
		if(ch == EOF) {
			my_perror();
			break;
		}
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
