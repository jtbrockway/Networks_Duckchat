#include "raw.h"
#include "duckchat.h"

#include <sys/types.h>
#include <sys/socket.h>

#include <pthread.h>

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
int sockfd;
char currentChannel[CHANNEL_MAX];
char activeChannels[1024][CHANNEL_MAX];

//Instantiate all requests
struct sockaddr_in serv_addr;
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
	free(say_txt);
	free(list_txt);
	free(who_txt);
	free(error_txt);
	return;
}

//Function that handles recieving anything from the server
void *recieveThread(void *){
	char rcvMsg[4096];
	socklen_t fromLen;
	fromLen = sizeof(serv_addr);

	while(1){
		text_t inType;
		recvfrom(sockfd, rcvMsg, sizeof(rcvMsg), 0, (struct sockaddr *)&serv_addr, &fromLen);
		inType = ((text *)rcvMsg)->txt_type;
		
		//Handle Say text
		if(inType == 0){
			say_txt = (text_say *)rcvMsg;
			
			printf("[%s][%s]: %s", say_txt->txt_channel, say_txt->txt_username, say_txt->txt_text);
		}
		
		//Handle List text
		if(inType == 1){
			list_txt = (text_list *)rcvMsg;

			printf("Existing Channels:\n");
			int i;
			for(i = 0; i < list_txt->txt_nchannels; i++){
				char *channel = list_txt->txt_channels[i].ch_channel;
				printf(" %s\n", channel);
			}
		}

		//Handle Who text
		if(inType == 2){
			who_txt = (text_who *)rcvMsg;

			printf("Users on channel %s:\n", who_req->req_channel);
			int i;
			for(i = 0; i < who_txt->txt_nusernames; i++){
				printf(" %s\n", who_txt->txt_users[i].us_username);
			}
		}

		//Handle Error text
		if(inType == 3){
			error_txt = (text_error *)rcvMsg;
			
			printf("Client ERROR: %s\n", error_txt -> txt_error);
		}
	}

	return NULL;
}

int main(int argc, char *argv[]){
	//NULL out activeChannels
	int i;
	for(i = 0; i < CHANNEL_MAX; i++){
		strcpy(activeChannels[i], "0");
	}
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
	
	strcpy(currentChannel, "Common");
	strcpy(activeChannels[0], "Common");

	//Begin recieving thread
	pthread_t rcvThread;
	pthread_create(&rcvThread, NULL, &recieveThread, NULL);

	raw_mode();
	//Continuously parse input
	while(1){
		char input[100];
		char tokenCopy[100];
		if(fgets(input, 100, stdin) != NULL){
			//Tokenize the input
			char *token;
			char first;
			strcpy(tokenCopy, input);
			token = strtok(tokenCopy, " ");
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
					channel[strlen(channel) - 1] = 0;

					strncpy(join_req->req_channel, channel, CHANNEL_MAX - 1);
					sendto(sockfd, join_req, sizeof(request_join), 0, (struct sockaddr *)&serv_addr, sizeof(serv_addr));
					
					//Add to active channels
					strcpy(currentChannel, channel);
					int j;
					for(j = 0; j < CHANNEL_MAX; j++){
						if(strcmp(activeChannels[j], "0") == 0){
							strcpy(activeChannels[j], channel);
							break;
						}
					}
				}

				//Handle Leave Request
				if(strcmp(&token[0], "/leave") == 0){
					char *channel = &token[7];
					if(strlen(channel) > 64){
						perror("Client: Channel name too long");
						exit(EXIT_FAILURE);
					}
					channel[strlen(channel) - 1] = 0;
					strncpy(leave_req->req_channel, channel, CHANNEL_MAX - 1);

					sendto(sockfd, leave_req, sizeof(request_leave), 0, (struct sockaddr *)&serv_addr, sizeof(serv_addr));

					int j;
					for(j = 0; j < CHANNEL_MAX; j++){
						if(strcmp(channel, activeChannels[j]) == 0){
							strcpy(activeChannels[j], "0");
							break;
						}
					}
				}

				//Handle List request
				if(strcmp(&token[0], "/list\n") == 0){
					sendto(sockfd, list_req, sizeof(request_list), 0, (struct sockaddr *)&serv_addr, sizeof(serv_addr));
				}

				//Handle Who request
				if(strcmp(&token[0], "/who") == 0){
					char *channel = &token[5];
					if(strlen(channel) > 64){
						perror("Client: Channel name too long");
						exit(EXIT_FAILURE);
					}
					channel[strlen(channel) - 1] = 0;
	
					strncpy(who_req->req_channel, channel, CHANNEL_MAX - 1);

					sendto(sockfd, who_req, sizeof(request_who), 0, (struct sockaddr *)&serv_addr, sizeof(serv_addr));
				}

				//Handle Switch request
				if(strcmp(&token[0], "/switch") == 0){
					char *channel = &token[8];
					if(strlen(channel) > 64){
						perror("Client: Channel name too long");
						exit(EXIT_FAILURE);
					}
					channel[strlen(channel) - 1] = 0;
					int nonActive = 0;
					int j;
					for(j = 0; j < CHANNEL_MAX; j++){
						if(strcmp(channel, activeChannels[j]) == 0){

							strcpy(currentChannel, channel);
							nonActive = 1;
							break;
						}
					}
					if(!nonActive){
						printf("Client: You are not subscribed to that Channel\n");
					}
				}	

				//Handle exit request
				if(strcmp(&token[0], "/exit\n") == 0){
					//Logout
					sendto(sockfd, logout_req, sizeof(request_logout), 0, (struct sockaddr *)&serv_addr, sizeof(serv_addr));
					break;
				}
			}else{
				//Send the users message
				strcpy(say_req->req_channel, currentChannel);
				strncpy(say_req->req_text, input, SAY_MAX);
				sendto(sockfd, say_req, sizeof(request_say), 0, (struct sockaddr *)&serv_addr, sizeof(serv_addr));
			}
		}
	}

	cooked_mode();
	pthread_cancel(rcvThread);
	clear_mem();
	shutdown(sockfd, SHUT_RDWR);
	return 0;
}
