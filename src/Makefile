# src/Makefile
CC = gcc
CFLAGS = -Wall -Wextra -O2 -std=c17

all: rush

rush: rush.c
	$(CC) $(CFLAGS) -o rush rush.c

clean:
	rm -f rush

.PHONY: all clean
