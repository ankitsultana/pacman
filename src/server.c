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

const int MAX_HANDLE_LEN = 30;
const int MIN_PLAYERS_PER_GAME = 3;
const int MAX_PLAYERS_PER_GAME = 5;
const int MAX_PLAYERS = 65536;
const int MAX_GAMES = 10;
const int MAX_MSG_SZ = 128;
char buffer[128] = {};

typedef int data_t;

typedef struct{
	char * handle;
	int handle_len;
	int pid;
	int gid;
	data_t dick_size;
}player_t;


typedef struct{
	int gid;
	int num_players;
	int * players;
}gameroom_t;

gameroom_t ** game_set;
player_t ** player_set;

player_t * get_new_player(int pid, int gid, char * handle, data_t dick_size){
	player_t * new_player = (player_t*)malloc(sizeof(player_t));

	new_player->handle = (char*)malloc(MAX_HANDLE_LEN*sizeof(char));
	memset(handle,0,MAX_HANDLE_LEN*sizeof(char));
	new_player->pid = pid;
	new_player->gid = gid;
	strcpy(new_player->handle,handle);
	new_player->handle_len = strlen(handle);
	new_player->dick_size = dick_size;

	return new_player;
}

// TODO: Write more efficient data structure for game_set so that this operation is optimized
int get_first_empty_game(gameroom_t ** game_set){
	for(int i=0; i<MAX_GAMES; i++)
		if(game_set[i] == NULL) return i;
}

gameroom_t * get_new_game(int gid, int max_players){
	gameroom_t * new_game = (gameroom_t*)malloc(sizeof(gameroom_t));
	new_game->gid = gid;
	new_game->num_players = 0;
	new_game->players = (int*)malloc(sizeof(int)*max_players);
	game_set[new_game->gid] = new_game;
	for(int i=0; i<max_players; i++) new_game->players[i] = -1;
	return new_game;
}

void start_new_game(gameroom_t * game){
	printf("This is game %d. process id = %d, parent process id = %d\n",game->gid,getpid(),getppid());
	while(1);
}

void init_player_set(){
	player_set = (player_t**)malloc(MAX_PLAYERS*sizeof(player_t*));
	for(int i=0; i<MAX_PLAYERS; i++) player_set[i] = NULL;
}

void init_game_set(){
	game_set = (gameroom_t**)malloc(MAX_GAMES*sizeof(gameroom_t*));
	for(int i=0; i<MAX_GAMES; i++) game_set[i] = NULL;
}

int init_listening_port(const char * port_number){
	struct addrinfo specs, * addrinfo_ll = NULL;
	memset(&specs,0,sizeof(specs));
	specs.ai_family = AF_INET;
	specs.ai_flags = AI_PASSIVE;
	specs.ai_socktype = SOCK_STREAM;
	if(getaddrinfo(NULL,"6969",&specs,&addrinfo_ll)<0){
		perror("getaddrinfo failed");
		exit(0);
	}
	int listening_port = socket(AF_INET,SOCK_STREAM,0);
	if(listening_port < 0){
		perror("socket failed\n");
		exit(0);
	}
	if(bind(listening_port,addrinfo_ll->ai_addr,addrinfo_ll->ai_addrlen)<0){
		perror("bind failed\n");
		exit(0);
	}
	freeaddrinfo(addrinfo_ll);
	return listening_port;
}

player_t * wait_for_new_player(int listening_port, int gid){
	struct sockaddr_storage client_addr;
	socklen_t addr_sz;
	char handle[30];
	data_t dick_size;
	int pid;
	player_t * new_player;
	memset(&client_addr,0,sizeof client_addr);
	memset(&addr_sz,0,sizeof addr_sz);
	memset(buffer,0,sizeof buffer);
	printf("Waiting for player\n");
	if(listen(listening_port,20) < 0){
		perror("listen failed\n");
		exit(0);
	}
	pid = accept(listening_port,(struct sockaddr*)(&client_addr),&addr_sz);
	if(pid < 0){
		perror("accept failed\n");
		exit(0);
	}
	printf("Player connected!\n");
	if(recv(pid,buffer,sizeof buffer,0) < 0){
		perror("recv failed\n");
		exit(0);
	}
	printf("%s\n",buffer);
	sscanf(buffer,"%s%d",handle,&dick_size);
	new_player = get_new_player(pid,gid,handle,dick_size);
	return new_player;
}

int main(int argc, char ** argv){

	init_player_set();
	init_game_set();
	int listening_port = init_listening_port("6969");
	
	while(1){
		
		int gid = get_first_empty_game(game_set);
		gameroom_t * new_game = get_new_game(gid,2);

		for(int pl = 1; pl <= 2; pl ++){

			player_t * new_player = wait_for_new_player(listening_port,gid);
			player_set[new_player->pid] = new_player;
			new_game->players[new_game->num_players++] = new_player->pid;
		}

		int proc_id = fork();
		if(proc_id==0){
			start_new_game(new_game);
		}

	}
	return 0;
}
