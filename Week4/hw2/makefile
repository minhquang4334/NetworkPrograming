CFLAGS = -c -Wall
CC = gcc
LIBS =  -lm

all: ser cli

cli: client.c
	 ${CC} -o client.o client.c

ser: server.c
	 ${CC} -o server.o server.c


clean:
	rm -f *.o *~