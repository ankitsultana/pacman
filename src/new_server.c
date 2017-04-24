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

#include "util.h"
#include "networking.h"
#include "game_state.h"
#include "player.h"

const char usage_fmt[] = "usage: %s server_port\n";

FILE * error_log;
const char* argv0;
int sockfd_listen;

void cry_usage(){
	fprintf(error_log, usage_fmt, argv0);
	exit(2);
}

void * listener_thread_func(void * arg) {
	// listen on listening socket wiht a maximum of 5 connections queued
	while(listen(sockfd_listen, 5) == 0) {
		// establish connection with the new player
		struct sockaddr_storage client_addr;
		int sockfd_new_client = accept(sockfd_listen, (struct sockaddr *) &client_addr, sizeof(client_addr)); 
		int sockifp_new_client = fdopen(sockfd_new_client, "r");
		int sockofp_new_client = fdopen(sockfd_new_client, "w");
		setvbuf(sockofp_new_client, NULL, _IONBF, BUFSIZ);
		print_sock_info(sockfd_new_client, error_log);

		// get the username
		

		// check if the username already exists
		
		// if username exists then reply with taken
		// else reply with accept

	}
}

int main(int argc, char ** argv) {
	// open error logfile
	error_log = fopen("../secret/server.error.log", "w");

	// parse command line args
	argv0 = argv[0];
	if(argc != 2) {
		cry_usage();
	}
	int port_no = atoi(argv[1]);

	// open up a socket and bind port_no to it
	sockfd_listen = get_server_socket(port_no);
	// get listening file descriptor
	// sockifp_listen = fdopen(sockfd_listen, "r");

	pthread_t listener_thread_id;
	pthread_create(&listener_thread_id, NULL, listener_thread_func, NULL);


	// listen to incoming requests
	return 0;
}
