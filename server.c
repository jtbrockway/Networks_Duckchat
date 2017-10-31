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
int SERVER_PORT;
int sockfd;

struct sockaddr_in serv_addr;
struct sockaddr_in cli_addr;
int clilen;

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

		//Parse input
		if(argc != 3){
				perror("Please enter: Host address and Host Port");
				exit(EXIT_FAILURE);
		}

		SERVER_HOST_IP_ADDRESS = argv[1];
		getHost(SERVER_HOST_IP_ADDRESS, ip);
		if((SERVER_PORT = atoi(argv[2])) == 0){
			perror("Server: Bad port: Please input an integer for port");
			exit(EXIT_FAILURE);
		}

		if((sockfd = socket(AF_INET, SOCK_DGRAM, 0)) < 0){
			perror("Server: Cant open datagram socket");
		}

		bzero((char *)&serv_addr, sizeof(serv_addr));
		serv_addr.sin_family = AF_INET;
		serv_addr.sin_addr.s_addr = inet_addr(ip);
		serv_addr.sin_port = htons(SERVER_PORT);

		if(bind(sockfd, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0){
			perror("Server: Cannot bind local address");
		}
		listen(sockfd, 10);

		int newsockfd;
		char rcvMsg[4096];
		socklen_t fromLen;
		fromLen = sizeof(cli_addr);
		//Continuously listen to clients
		while(1){
			newsockfd = accept(sockfd, (struct sockaddr *)&cli_addr, (socklen_t *)&clilen);

			request_t reqType;
			recvfrom(newsockfd, rcvMsg, sizeof(rcvMsg), 0, (struct sockaddr *)&serv_addr, &fromLen);
			reqType = ((request *)rcvMsg)->req_type;

			if(reqType == 0){
				printf("LOGIN");
			}
			if(reqType == 2){
				printf("JOIN");
			}
		}
		return 0;
}
