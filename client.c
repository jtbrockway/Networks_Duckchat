#include "raw.h"
#include "duckchat.h"

#include <sys/types.h>
#include <sys/socket.h>

#include <string.h>
#include <strings.h>
#include <arpa/inet.h>

#include <stdlib.h>
#include <stdio.h>

char *SERVER_HOST_IP_ADDRESS;
char *USERNAME;
int  SERVER_PORT;

//Instantiate all requests
struct sockaddr_in serv_addr;
struct request_login *login_req   = (request_login *)calloc(0, sizeof(request_login));
struct request_logout *logout_req = (request_logout *)calloc(0, sizeof(request_logout));
struct request_join *join_req     = (request_join *)calloc(0, sizeof(request_join));
struct request_leave *leave_req   = (request_leave *)calloc(0, sizeof(request_leave));
struct request_say *say_req       = (request_say *)calloc(0, sizeof(request_say));
struct request_list *list_req     = (request_list *)calloc(0, sizeof(request_list));
struct request_who *who_req       = (request_who *)calloc(0, sizeof(request_who));

void clear_mem(){
	free(login_req);
	free(logout_req);
	free(join_req);
	free(leave_req);
	free(say_req);
	free(list_req);
	free(who_req);
	return;
}

int main(int argc, char *argv[]){
	//Assign request codes
	login_req -> req_type = 0;
	logout_req -> req_type = 1;
	join_req -> req_type = 2;
	leave_req -> req_type = 3;
	say_req -> req_type = 4;
	list_req -> req_type = 5;
	who_req -> req_type = 6;

	//Parse input
	if(argc != 4){
			perror("Please input: Host name of server, Port number of server, and your Username\n");
			exit(EXIT_FAILURE);
	}

	SERVER_HOST_IP_ADDRESS = argv[1];
	if((SERVER_PORT = atoi(argv[2])) == 0){
			perror("Bad port: Please input an integer for port\n");
			exit(EXIT_FAILURE);
	}
	USERNAME = argv[3];

	//Specify server info
	bzero((char *)&serv_addr, sizeof(serv_addr));
	serv_addr.sin_family = AF_INET;
	serv_addr.sin_addr.s_addr = inet_addr(SERVER_HOST_IP_ADDRESS);
	serv_addr.sin_port = htons(SERVER_PORT);

	//Create socket for client
	int sockfd;
	if((sockfd = socket(AF_INET, SOCK_DGRAM, 0)) < 0){
			perror("Client: cannot open datagram socket\n");
	}

	//Connect to server socket
	if(connect(sockfd, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0){
			perror("Client: cannot connect to server\n");
	}

	//Check username is size appropriate
	if(sizeof(USERNAME) > 32){
			perror("Username exceeds 32 bits");
			exit(EXIT_FAILURE);
	}

	//Send initial login reuqest
	strncpy(login_req->req_username, USERNAME, USERNAME_MAX - 1);
	strncpy(join_req->req_channel, "Common", CHANNEL_MAX - 1);
	if(sendto(sockfd, login_req, sizeof(login_req), 0, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0){
			perror("Client: Login failed");
			exit(EXIT_FAILURE);
	}
	if(sendto(sockfd, join_req, sizeof(join_req), 0, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0){
			perror("Client: Joining (Common) failed");
			exit(EXIT_FAILURE);
	}

	clear_mem();
	return 0;
}
