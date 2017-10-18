#include "raw.h"
#include "duckchat.h"

#include <sys/types.h>
#include <sys/socket.h>

#include <strings.h>
#include <arpa/inet.h>

#include <stdlib.h>
#include <stdio.h>

char *SERVER_HOST_IP_ADDRESS;
char *USERNAME;
int  SERVER_PORT;

struct sockaddr_in serv_addr;

int main(int argc, char *argv[]){

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

		
		return 0;
}
