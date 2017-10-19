#include "duckchat.h"

#include <sys/types.h>
#include <sys/socket.h>

#include <strings.h>
#include <arpa/inet.h>

#include <stdlib.h>
#include <stdio.h>

int main(int argc, char *argv[]){

		//Parse input
		if(argc != 3){
				perror("Please enter: Host address and Host IP");

				exit(EXIT_FAILURE);
		}
		return 0;
}
