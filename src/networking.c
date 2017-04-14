#define POSIX_SOURCE
#include <stdio.h>
#include <time.h>
#include <assert.h>
#include <stdlib.h>
#include <string.h>

#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netinet/tcp.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <signal.h>
#include <sys/wait.h>

#include "util.h"
#include "networking.h"

void set_sockaddr_in(struct sockaddr_in* p_saddr, const char* ipaddr, int port)
{
	p_saddr->sin_family = AF_INET;
	p_saddr->sin_port = htons(port);
	p_saddr->sin_addr.s_addr = inet_addr(ipaddr);
	memset(p_saddr->sin_zero, '\0', sizeof (p_saddr->sin_zero));
}

void parse_sockaddr_in(struct sockaddr_in* p_saddr, char* ipaddr, int* p_port)
{
	*p_port = ntohs(p_saddr->sin_port);
	strcpy(ipaddr, inet_ntoa(p_saddr->sin_addr));
}

void set_socket_option(int sockfd, int level, int option)
{
	int on=1, err;
	err = setsockopt(sockfd, level, option, &on, sizeof on);
	if(err == -1)
		my_perror();
}

int get_client_socket(const char* ipaddr, int port)
{
	int sockfd, err;
	struct sockaddr_in addr;
	sockfd = socket(PF_INET, SOCK_STREAM, 0);
	if(sockfd == -1) {
		my_perror();
		return 1;
	}
	set_socket_option(sockfd, SOL_SOCKET, SO_REUSEADDR);
	set_socket_option(sockfd, IPPROTO_TCP, TCP_NODELAY);
	set_sockaddr_in(&addr, ipaddr, port);
	err = connect(sockfd, (struct sockaddr*)&addr, sizeof addr);
	if(err == -1) {
		my_perror();
		return 1;
	}
	return sockfd;
}

void get_sock_info(int sockfd, char* my_ipaddr, int* p_my_port, char* peer_ipaddr, int* p_peer_port)
{
	int err;
	socklen_t addr_len;
	struct sockaddr_in my_addr, peer_addr;
	memset(&my_addr, 0, sizeof my_addr);
	memset(&peer_addr, 0, sizeof peer_addr);

	addr_len = sizeof my_addr;
	err = getsockname(sockfd, (struct sockaddr*)&my_addr, &addr_len);
	if(err == -1)
		my_perror();
	parse_sockaddr_in(&my_addr, my_ipaddr, p_my_port);

	addr_len = sizeof peer_addr;
	err = getpeername(sockfd, (struct sockaddr*)&peer_addr, &addr_len);
	if(err == -1)
		my_perror();
	parse_sockaddr_in(&peer_addr, peer_ipaddr, p_peer_port);
}

void print_sock_info(int sockfd, FILE* stream)
{
	char my_ipaddr[20], peer_ipaddr[20];
	int my_port, peer_port;
	get_sock_info(sockfd, my_ipaddr, &my_port, peer_ipaddr, &peer_port);
	fprintf(stream, "Connected to %s:%d via %s:%d\n", peer_ipaddr, peer_port, my_ipaddr, my_port);
}
