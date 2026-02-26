# Name: Lucan Akpati		
# NSID: jbs671
# Student Number: 11331253

.PHONY: all clean


CC = gcc
CFLAGS = -g -O0 -Wall -pedantic -Wextra -std=gnu90 -I.


all: d_vec

clean:
	rm -f *.o d_vec

#########################################################

d_vec.o: d_vec.c
	$(CC) -c d_vec.c $(CFLAGS)

d_vec: d_vec.o 
	$(CC) $(CFLAGS) d_vec.o -o d_vec

