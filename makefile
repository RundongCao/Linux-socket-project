
CC = g++
DUBUG = -g
CFLAGS = -Wall -c $(DEBUG)
LFLAGS = -Wall $(DEBUG)

all:server_orserver server_andserver edgeserver client

server_or:
	./server_orserver

server_and:
	./server_andserver

edge: 
	./edgeserver

server_orserver: server_or.o
	$(CC) $(LFLAGS) server_or.o -o server_orserver

server_andserver: server_and.o
	$(CC) $(LFLAGS) server_and.o -o server_andserver

edgeserver: edge.o
	$(CC) $(LFLAGS) edge.o -o edgeserver

client: client.o
	$(CC) $(LFLAGS) client.o -o client

server_or.o: server_or.c
	$(CC) $(CFLAGS) server_or.c

server_and.o: server_and.c
	$(CC) $(CFLAGS) server_and.c

edge.o: edge.c
	$(CC) $(CFLAGS) edge.c

client.o: client.c
	$(CC) $(CFLAGS) client.c


clean:
	\rm *.o server_orserver server_andserver edgeserver client

