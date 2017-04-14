#define _POSIX_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>

#include <ncurses.h>
#include <pthread.h>

#include "networking.h"

const char usage_fmt[] = "usage: %s server_ip server_port username\n";

const char app_name[] = "pacmen";
const char* argv0;

void cry_usage() {
	fprintf(stderr, usage_fmt, argv0);
	exit(2);
}

#define BUFSIZE 2048

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
		char ch = getchar();
		send_char(ch);
	}
	return NULL;
}

void execute_message(char* message) {
	fprintf(stderr, "Message:\n%s", buffer);
}

void* receiver_thread_func(void* arg) {
	int offset = 0;
	while(fgets(buffer+offset, BUFSIZE, sockifp) != NULL) {
		bool empty_line = (buffer[offset] == '\n');
		while(buffer[offset] != '\0') {
			offset++;
		}
		if(empty_line) {
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

	// create threads
	pthread_t sender_thread_id;
	pthread_create(&sender_thread_id, NULL, sender_thread_func, NULL);
	receiver_thread_func(NULL);

	return 0;
}
