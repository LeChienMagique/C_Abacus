CC=gcc
CFLAGS=-Wall -Werror -Wextra -std=c11 -pedantic
LDFLAGS=
LDLIBS=-lm

OBJ = ./src/token.o ./src/ast.o ./src/ast_operations.o ./src/runtime.o
OBJ_DEBUG = debug.o ${OBJ}
OBJ_TEST = test.o ${OBJ}
OBJ_CRIT_TEST = crit_tests.o ${OBJ}

all: abacus

abacus: ${OBJ} main.o
	${CC} ${CFLAGS} $^ ${LDLIBS} -o $@

debug: CFLAGS+=-g -fsanitize=address
debug: LDLIBS+=-fsanitize=address 
debug: ${OBJ_DEBUG}

check: CFLAGS+=-g -fsanitize=address -fprofile-arcs -ftest-coverage
check: LDLIBS+=-fsanitize=address -lgcov
check: ${OBJ_TEST}
check:
	$(CC) $(CFLAGS) $^ $(LDLIBS) -o $@
	./check -v
	gcovr --html report.html --html-nested --html-syntax-highlighting

# check: CFLAGS+=-fprofile-arcs -ftest-coverage -g -fsanitize=address -lcriterion
# check: LDLIBS+=-fsanitize=address -lcriterion
# check: $(OBJ_CRIT_TEST)
# check:
# 	$(CC) $(CFLAGS) $^ $(LDLIBS) -o $@
# 	./$@ --verbose --always-succeed
# 	gcovr --html report.html --html-nested --html-syntax-highlighting

.PHONY: clean

clean:
	${RM} abacus check debug
	${RM} *.gc* src/*.gc* report.*
	${RM} ${OBJ} ${OBJ_TEST} ${OBJ_DEBUG} main.o
