CC=g++

CFLAGS=-Wall -W -g -Werror 


all: client server

client: client.c raw.c
	$(CC) client.c raw.c -lpthread $(CFLAGS) -o client

server: server.c 
	$(CC) server.c $(CFLAGS) -o server

clean:
	rm -f client server *.o
