CC=gcc
CFLAGS=-Wall -Wextra -g -std=c11 -pedantic
LDFLAGS=
LDLIBS=-lm

main: main.c token.c ast.c test.c ast_operations.c

main.c:
ast.c:
token.c:
test.c:
ast_operations.c:

clean:
	${RM} main
