# make file for building single source file programs.

CC = gcc
CFLAGS = -Wall
VPATH = src

programs = snake

all: $(programs)

clean:
	rm -f $(programs)
