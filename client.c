#include "raw.h"
#include "duckchat.h"

#include <sys/types.h>
#include <sys/socket.h>

#include <string.h>
#include <strings.h>
#include <arpa/inet.h>
#include <netdb.h>

#include <stdlib.h>
#include <stdio.h>

char *SERVER_HOST_IP_ADDRESS;
char ip[100];
char *USERNAME;
int  SERVER_PORT;

//Instantiate all requests
struct sockaddr_in serv_addr;
struct request_login *login_req;
struct request_logout *logout_req;
struct request_join *join_req;
struct request_leave *leave_req;
struct request_say *say_req;
struct request_list *list_req;
struct request_who *who_req;

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
	//Initialize requests to 0
	memset(login_req, 0, sizeof(request_login));
	memset(logout_req, 0, sizeof(request_logout));
	memset(join_req, 0, sizeof(request_join));
	memset(leave_req, 0, sizeof(request_leave));
	memset(say_req, 0, sizeof(request_say));
	memset(list_req, 0, sizeof(request_list));
	memset(who_req, 0, sizeof(request_who));
	//Set request codes
	login_req -> req_type = 0;
	logout_req -> req_type = 1;
	join_req -> req_type = 2;
	leave_req -> req_type = 3;
	say_req -> req_type = 4;
	list_req -> req_type = 5;
	who_req -> req_type = 6;
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
	//Allocate and instantiate requests
	create_requests();

	//Parse input
	if(argc != 4){
			perror("Please input: Host name of server, Port number of server, and your Username\n");
			exit(EXIT_FAILURE);
	}

	SERVER_HOST_IP_ADDRESS = argv[1];
	getHost(SERVER_HOST_IP_ADDRESS, ip);
	if((SERVER_PORT = atoi(argv[2])) == 0){
			perror("Bad port: Please input an integer for port\n");
			exit(EXIT_FAILURE);
	}
	USERNAME = argv[3];
	//Check username is size appropriate
	if(sizeof(USERNAME) > 32){
			perror("Username exceeds 32 bits");
			exit(EXIT_FAILURE);
	}

	//Specify server info
	bzero((char *)&serv_addr, sizeof(serv_addr));
	serv_addr.sin_family = AF_INET;
	serv_addr.sin_addr.s_addr = inet_addr(ip);
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

	//Send initial login reuqest
	strncpy(login_req->req_username, USERNAME, USERNAME_MAX - 1);
	strncpy(join_req->req_channel, "Common", CHANNEL_MAX - 1);
	if(sendto(sockfd, login_req, sizeof(request_login), 0, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0){
			perror("Client: Login failed");
			exit(EXIT_FAILURE);
	}
	if(sendto(sockfd, join_req, sizeof(request_join), 0, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0){
			perror("Client: Joining (Common) failed");
			exit(EXIT_FAILURE);
	}
	
	//Continuously parse input
	char input[100];
	while(1){
		if(fgets(input, 100, stdin) != NULL){
			//Tokenize the input
			//char *inputCopy = input;
			char *token;
			char first;
			token = strtok(input, " ");
			first = *token;

			//If theres a request
			if(first == '/'){

				//Handle Join request
				if(strcmp(&token[0], "/join") == 0){
					char *channel = &token[6];
					if(strlen(channel) > 64){
						perror("Client: Channel name too long");
						exit(EXIT_FAILURE);
					}
					strncpy(join_req->req_channel, channel, CHANNEL_MAX - 1);

					sendto(sockfd, join_req, sizeof(request_join), 0, (struct sockaddr *)&serv_addr, sizeof(serv_addr));
				}

				//Handle Leave Request
				if(strcmp(&token[0], "/leave") == 0){
					char *channel = &token[7];
					if(strlen(channel) > 64){
						perror("Client: Channel name too long");
						exit(EXIT_FAILURE);
					}
					strncpy(leave_req->req_channel, channel, CHANNEL_MAX - 1);

					sendto(sockfd, leave_req, sizeof(request_leave), 0, (struct sockaddr *)&serv_addr, sizeof(serv_addr));
				}

				//Handle List request
				if(strcmp(&token[0], "/list") == 0){
					sendto(sockfd, list_req, sizeof(request_list), 0, (struct sockaddr *)&serv_addr, sizeof(serv_addr));
					//FIX : NEED TO HANDLE RECIEVING THE LIST!!!!!!!!!
				}

				//Handle Who request
				if(strcmp(&token[0], "/who") == 0){
					char *channel = &token[4];
					if(strlen(channel) > 64){
						perror("Client: Channel name too long");
						exit(EXIT_FAILURE);
					}
					strncpy(who_req->req_channel, channel, CHANNEL_MAX - 1);

					sendto(sockfd, who_req, sizeof(request_who), 0, (struct sockaddr *)&serv_addr, sizeof(serv_addr));
					//FIX : NEED TO HANDLE RECIEVING THE WHO!!!!!!!!!
				}

				//Handle Switch request
				if(strcmp(&token[0], "/switch") == 0){
					char *channel = &token[8];
					if(strlen(channel) > 64){
						perror("Client: Channel name too long");
						exit(EXIT_FAILURE);
					}
					strncpy(join_req->req_channel, channel, CHANNEL_MAX - 1);

					sendto(sockfd, join_req, sizeof(request_join), 0, (struct sockaddr *)&serv_addr, sizeof(serv_addr));
				}

				//Handle exit request
				if(strcmp(&token[0], "/exit\n") == 0){
					//Logout
					sendto(sockfd, logout_req, sizeof(request_logout), 0, (struct sockaddr *)&serv_addr, sizeof(serv_addr));
					break;
				}
			}
		}
	}

	clear_mem();
	shutdown(sockfd, SHUT_RDWR);
	return 0;
}
