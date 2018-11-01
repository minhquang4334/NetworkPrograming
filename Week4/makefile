CFLAGS = -c -Wall
CC = gcc
LIBS =  -lm

all: ser cli

cli: udp-client-sample.c
	 ${CC} -o client.o udp-client-sample.c

ser: udp-server-sample.c
	 ${CC} -o server.o udp-server-sample.c


clean:
	rm -f *.o *~