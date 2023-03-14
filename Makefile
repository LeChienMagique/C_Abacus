CC=gcc
CFLAGS=-Wall -Wextra -g -std=c11 -pedantic
LDFLAGS=
LDLIBS=-lm

SRC = main.c ./src/token.c ./src/ast.c ./src/test.c ./src/ast_operations.c
all: main

main: ${SRC}

.PHONY: clean

clean:
	${RM} main
