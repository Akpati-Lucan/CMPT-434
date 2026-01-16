# Name: Lucan Akpati		
# NSID: jbs671
# Student Number: 11331253

.PHONY: all clean


CC = gcc
CFLAGS = -g -O0 -Wall -pedantic -Wextra -std=gnu90 -I.


all: 
	client server

clean:
	rm -f *.o client server

#########################################################

client.o: client.c
	$(CC) -c client.c $(CFLAGS)

client: client.o 
	$(CC) $(CFLAGS) client.o -o client 


#########################################################

server.o: server.c
	$(CC) -c server.c $(CFLAGS)

server: server.o 
	$(CC) $(CFLAGS) server.o -o server 
