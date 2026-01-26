# Name: Lucan Akpati		
# NSID: jbs671
# Student Number: 11331253

.PHONY: all clean


CC = gcc
CFLAGS = -g -O0 -Wall -pedantic -Wextra -std=gnu90 -I.


all: client server_without_proxy

clean:
	rm -f *.o client server_without_proxy

#########################################################

client.o: client.c
	$(CC) -c client.c $(CFLAGS)

client: client.o 
	$(CC) $(CFLAGS) client.o -o client 


#########################################################

server_without_proxy.o: server_without_proxy.c
	$(CC) -c server_without_proxy.c $(CFLAGS)

server_without_proxy: server_without_proxy.o 
	$(CC) $(CFLAGS) server_without_proxy.o -o server_without_proxy
