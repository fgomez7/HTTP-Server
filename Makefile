CC = clang
CFLAGS = -Wall -Wextra -Werror -Wpedantic

all: httpserver

httpserver: httpserver.o bind.o
	$(CC) -o httpserver httpserver.o bind.o

httpserver.o: httpserver.c
	$(CC) $(CFLAGS) -c httpserver.c

bind.o: bind.c bind.h
	$(CC) $(CFLAGS) -c bind.c

clean:
	rm -f httpserver httpserver.o bind.o