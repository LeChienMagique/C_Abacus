CC=gcc
CFLAGS=-Wall -Wextra -g -std=c11 -pedantic
LDFLAGS=
LDLIBS=-lm

SRC = main.c ./src/token.c ./src/ast.c ./src/test.c ./src/ast_operations.c ./src/errors.c

all: main

main:
	${CC} ${CFLAGS} ${SRC} ${LDLIBS} -o abacus

.PHONY: clean

clean:
	${RM} abacus
