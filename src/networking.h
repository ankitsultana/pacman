#ifndef H_NETWORKING
#define H_NETWORKING

#define POSIX_SOURCE
#include <netinet/in.h>

int get_client_socket(const char* ipaddr, int port);

void get_sock_info(int sockfd, char* my_ipaddr, int* p_my_port, char* peer_ipaddr, int* p_peer_port);

void print_sock_info(int sockfd, FILE* stream);

int get_server_socket(int port);

#endif	// H_NETWORKING
