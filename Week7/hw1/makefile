CFLAGS = -c -Wall
CC = gcc
LIBS =  -lm

all: server client

server: server.o protocol.o
	${CC} -pthread server.o protocol.o -o server

client: client.o protocol.o
	${CC} client.o protocol.o -o client

server.o: tcp-server.c
	${CC} ${CFLAGS} -o server.o tcp-server.c

client.o: tcp-client.c
	${CC} ${CFLAGS} -o client.o tcp-client.c

protocol: protocol.c
	${CC} ${CFLAGS} protocol.c

clean:
	rm -f *.o *~
	rm server
	rm client