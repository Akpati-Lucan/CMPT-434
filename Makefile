# Name: Lucan Akpati		
# NSID: jbs671
# Student Number: 11331253

.PHONY: all clean


CC = gcc
CFLAGS = -g -O0 -Wall -pedantic -Wextra -std=gnu90 -I.


all: sender receiver

clean:
	rm -f *.o sender receiver

#########################################################

sender.o: sender.c
	$(CC) -c sender.c $(CFLAGS)

sender: sender.o 
	$(CC) $(CFLAGS) sender.o -o sender 


#########################################################

receiver.o: receiver.c
	$(CC) -c receiver.c $(CFLAGS)

receiver: receiver.o 
	$(CC) $(CFLAGS) receiver.o -o receiver

