#include "raw.c"
#include "duckchat.h"
#include <sys/types.h>
#include <sys/socket.h>
#include <stdio.h>

int main(int argc, char *argv[]){
		//Parse input
		if(argc != 4){
				perror("Please input: Host name of server, 
								Port number of server, and your Username");
				return 0;
		}

		//Specify server info

		//Create socket for client
		int sockfd;
		if((sockfd = socket(AF_INET, SOCK_DGRAM, 0)) < 0){
				perror("client: can't open stream socket");
		}

		//Connect to server socket
		
		return 0;
}
