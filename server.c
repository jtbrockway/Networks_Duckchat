#include "duckchat.h"

#include <sys/types.h>
#include <sys/socket.h>

#include <string.h>
#include <strings.h>
#include <arpa/inet.h>
#include <netdb.h>

#include <stdlib.h>
#include <stdio.h>

//User Struct
struct User{
	char username[USERNAME_MAX];
	sockaddr_in user_addr;
};

//Channel Struct
struct Channel{
	char chanName[CHANNEL_MAX];
	struct User *chanUsers[4096];
};

struct User users[4096];
struct Channel channels[8192];

char *SERVER_HOST_IP_ADDRESS;
char ip[100];
int SERVER_PORT;
int sockfd;

struct sockaddr_in serv_addr;
struct sockaddr_in cli_addr;
int clilen;

struct request_login *login_req;
struct request_logout *logout_req;
struct request_join *join_req;
struct request_leave *leave_req;
struct request_say *say_req;
struct request_list *list_req;
struct request_who *who_req;
struct text_say *say_txt;
struct text_list *list_txt;
struct text_who *who_txt;
struct text_error *error_txt;

//Allocates space for requests and initializes them
void create_requests(){
	//Allocate space for requests
	login_req   = (request_login *)malloc(sizeof(request_login));
	logout_req = (request_logout *)malloc(sizeof(request_logout));
	join_req     = (request_join *)malloc(sizeof(request_join));
	leave_req   = (request_leave *)malloc(sizeof(request_leave));
	say_req       = (request_say *)malloc(sizeof(request_say));
	list_req     = (request_list *)malloc(sizeof(request_list));
	who_req       = (request_who *)malloc(sizeof(request_who));
	say_txt          = (text_say *)malloc(sizeof(text_say));
	list_txt        = (text_list *)malloc(sizeof(text_list));
	who_txt          = (text_who *)malloc(sizeof(text_who));
	error_txt      = (text_error *)malloc(sizeof(text_error));
	//Initialize requests to 0
	memset(login_req, 0, sizeof(request_login));
	memset(logout_req, 0, sizeof(request_logout));
	memset(join_req, 0, sizeof(request_join));
	memset(leave_req, 0, sizeof(request_leave));
	memset(say_req, 0, sizeof(request_say));
	memset(list_req, 0, sizeof(request_list));
	memset(who_req, 0, sizeof(request_who));
	memset(say_txt, 0, sizeof(text_say));
	memset(list_txt, 0, sizeof(text_list));
	memset(who_txt, 0, sizeof(text_who));
	memset(error_txt, 0, sizeof(text_error));
	//Set request codes
	login_req -> req_type = 0;
	logout_req -> req_type = 1;
	join_req -> req_type = 2;
	leave_req -> req_type = 3;
	say_req -> req_type = 4;
	list_req -> req_type = 5;
	who_req -> req_type = 6;
	say_txt -> txt_type = 0;
	list_txt -> txt_type = 1;
	who_txt -> txt_type = 2;
	error_txt -> txt_type = 3;
}

//Free allocated memory
void clear_mem(){
	free(login_req);
	free(logout_req);
	free(join_req);
	free(leave_req);
	free(say_req);
	free(list_req);
	free(who_req);
	free(say_txt);
	free(list_txt);
	free(who_txt);
	free(error_txt);
	return;
}

//Gets the IP address when given a domain name
//Obtained from binarytides.com
int getHost(char *hostname, char *ip){
	struct hostent *he;
	struct in_addr **addr_list;
	int i;

	if((he = gethostbyname(hostname)) == NULL){
		perror("Get hostname failed");
		return 1;
	}

	addr_list = (struct in_addr **)he->h_addr_list;
	for(i = 0; addr_list[i] != NULL; i++){
		strcpy(ip, inet_ntoa(*addr_list[i]));
		return 0;
	}
	return 1;
}

int main(int argc, char *argv[]){
	int i;
	for(i = 0; i < 4096; i++){
		strcpy(users[i].username, " ");
	}
	//Allocate Requests
	create_requests();

	//Parse input
	if(argc != 3){
			perror("Please enter: Host address and Host Port");
			exit(EXIT_FAILURE);
	}

	SERVER_HOST_IP_ADDRESS = argv[1];
	getHost(SERVER_HOST_IP_ADDRESS, ip);
	if((SERVER_PORT = atoi(argv[2])) == 0){
		perror("Server: Bad port: Please input an integer for port\n");
		exit(EXIT_FAILURE);
	}

	if((sockfd = socket(AF_INET, SOCK_DGRAM, 0)) < 0){
		perror("Server: Cant open datagram socket\n");
		exit(EXIT_FAILURE);
	}

	bzero((char *)&serv_addr, sizeof(serv_addr));
	serv_addr.sin_family = AF_INET;
	serv_addr.sin_addr.s_addr = inet_addr(ip);
	serv_addr.sin_port = htons(SERVER_PORT);

	if(bind(sockfd, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0){
		perror("Server: Cannot bind local address\n");
		exit(EXIT_FAILURE);
	}

	listen(sockfd, 10);

	User *currentUser = (User *)malloc(sizeof(User));;
	char rcvMsg[65536];
	socklen_t fromLen;
	fromLen = sizeof(cli_addr);
	//Continuously listen to clients
	while(1){
		if(recvfrom(sockfd, rcvMsg, sizeof(rcvMsg), 0, (struct sockaddr *)&cli_addr, &fromLen) < 0){
			continue;
		}

		currentUser->user_addr = cli_addr;

		request_t reqType;
		reqType = ((request *)rcvMsg)->req_type;

		//Handle LOGIN Request
		if(reqType == 0){
			login_req = (request_login *)rcvMsg;

			int i;
			for(i = 0; i < 4096; i++){
				if(strcmp(users[i].username, " ") == 0){
					strcpy(users[i].username, login_req->req_username);
					users[i].user_addr = cli_addr;
					break;
				}
			}

			printf("User %s has logged in\n", users[i].username);
		}
		
		//Set currentUser's Username
		int i;
		int userExists = 0;
		for(i = 0; i < 4096; i++){
			if((users[i].user_addr.sin_addr.s_addr == currentUser->user_addr.sin_addr.s_addr) && (users[i].user_addr.sin_port == currentUser->user_addr.sin_port)){
					strcpy(currentUser->username, users[i].username);
					userExists = 1;
					break;
			}
		}
		if(!userExists){
			continue;
		}
		
		//Handle LOGOUT request
		if(reqType == 1){
			printf("LOGOUT");
		}

		//Handle JOIN request
		if(reqType == 2){
			join_req = (request_join *)rcvMsg;

			int i;
			int exists = 0;
			for(i = 0; i < 8192; i++){
				if(strcmp(channels[i].chanName, join_req->req_channel) == 0){
					int j;
					int userExists = 0;
					for(j = 0; j < 4096; j++){
						if(strcmp(channels[i].chanUsers[j]->username, currentUser->username) == 0){
							userExists = 1;
							break;
						}
					}
					if(!userExists){
						for(j = 0; j < 4096; j++){
							if(strcmp(channels[i].chanUsers[j]->username, " ") == 0){
								strcpy(channels[i].chanUsers[j]->username, currentUser->username);
								channels[i].chanUsers[j]->user_addr = currentUser->user_addr;
							}
						}
					}
					exists = 1;
					break;
				}
			}
			if(!exists){
				for(i = 0; i < 8192; i++){
					if(strcmp(channels[i].chanName, " ") == 0){
						strcpy(channels[i].chanName, join_req->req_channel);
						channels[i].chanUsers[0] = currentUser;
					}
				}
			}
		}

		//Handle LEAVE request
		if(reqType == 3){
			printf("LEAVE");
		}
		
		//Handle SAY request
		if(reqType == 4){
			printf("SAY");
		}

		//Handle LIST request
		if(reqType == 5){
			printf("LIST");
		}
		
		//Handle WHO request
		if(reqType == 6){
			printf("WHO");
		}
	}

	free(currentUser);
	clear_mem();
	return 0;
}
