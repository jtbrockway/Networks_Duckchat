#include "duckchat.h"

#include <sys/types.h>
#include <sys/socket.h>

#include <strings.h>
#include <arpa/inet.h>

#include <stdlib.h>
#include <stdio.h>

char *SERVER_HOST_IP_ADDRESS;
int SERVER_PORT;

int main(int argc, char *argv[]){

		//Parse input
		if(argc != 3){
				perror("Please enter: Host address and Host Port");

				exit(EXIT_FAILURE);
		}


		return 0;
}
