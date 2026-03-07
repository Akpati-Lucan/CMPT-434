# Name: Lucan Akpati		
# NSID: jbs671
# Student Number: 11331253
#
# Name: Samuel Olukuewu
# NSID: nds091
# Student Number: 11323380

.PHONY: all clean

CC = gcc
CFLAGS = -g -O0 -Wall -pedantic -Wextra -std=gnu90 -I.

#####################################################################

all: raft
	
clean:
	rm -f *.o *.a raft
	
######################################################################


list_adders.o: list_adders.c list.h
	$(CC) $(CFLAGS) -c -o list_adders.o list_adders.c

list_movers.o: list_movers.c list.h
	$(CC) $(CFLAGS) -c -o list_movers.o list_movers.c

list_removers.o: list_removers.c list.h
	$(CC) $(CFLAGS) -c -o list_removers.o list_removers.c

liblist.a: list_adders.o list_movers.o list_removers.o
	ar rcs liblist.a list_adders.o list_movers.o list_removers.o



raft.o: raft.c 
	$(CC) $(CFLAGS) -c raft.c

raft: raft.o liblist.a 
	$(CC) $(CFLAGS)  raft.o -o raft -L. -llist

