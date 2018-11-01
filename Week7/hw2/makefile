CFLAGS = -c -Wall
CC = gcc
LIBS =  -lm

all: ser cli

cli: client.c
	 ${CC} -o client client.c

ser: server.c
	 ${CC} -o server server.c

clean:
	rm -f *.o *~