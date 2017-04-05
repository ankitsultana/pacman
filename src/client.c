#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <string.h>
#include <curses.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <netdb.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <unistd.h>

const int MAX_MSG_SZ = 128;
char buffer[128] = {0};

int start_new_game(){
	struct addrinfo specs, * addrinfo_ll;
	memset(&specs,0,sizeof(specs));
	specs.ai_family = AF_INET;
	specs.ai_socktype = SOCK_STREAM;
	if(getaddrinfo("127.0.0.1","6969",&specs,&addrinfo_ll)<0){
		perror("getaddrinfo failed\n");
		exit(0);
	}
	int sock_fd = socket(AF_INET,SOCK_STREAM,0);
	if(sock_fd < 0){
		perror("socket failed\n");
		exit(0);
	}
	if(connect(sock_fd,addrinfo_ll->ai_addr,addrinfo_ll->ai_addrlen)<0){
		perror("connect failed\n");
		exit(0);
	}
	freeaddrinfo(addrinfo_ll);
	return sock_fd;
}

void send_message(const int server, const char * message){
	memset(buffer,0,sizeof buffer);
	strcpy(buffer,message);
	if(send(server,buffer,strlen(buffer),0) < 0){
		perror("send failed\n");
		exit(0);
	}
}

void recv_message(const int server){
	memset(buffer,0,sizeof buffer);
	if(recv(server,buffer,sizeof buffer,0) < 0){
		perror("recv failed\n");
		exit(0);
	}
}

void leave_game(const int game_instance){
	send_message(game_instance,"I want to leave");
	return;
}

int main(int argc, char ** argv){
	if(argc != 3) {
		fprintf(stderr, "usage");
		assert(false);
	}
	int game_instance = start_new_game();
	char tmp[MAX_MSG_SZ];
	memset(tmp,0,sizeof tmp);
	strcpy(tmp,argv[1]);
	strcat(tmp," ");
	strcat(tmp,argv[2]);
	send_message(game_instance,tmp);
	//recv_message(game_instance);
	printf("%s\n",buffer);
	sleep(1);
	return 0;
}
