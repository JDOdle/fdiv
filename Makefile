CC = gcc
CFLAGS = -g

make: fdiv.c
	$(CC) $(CFLAGS) -o fdiv fdiv.c

install: fdiv
	cp fdiv /usr/local/bin/
