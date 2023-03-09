#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <err.h>
#include <assert.h>
#include <stdbool.h>

#include "./token.h"
#include "./ast.h"
#include "./test.h"

int evaluate_input(char* input) {
    Token* tokens = calloc(1, sizeof(Token));
    Token* sentinel = tokens;
    size_t index = 0;
    while (index < strlen(input)) {
        tokens->next = next_token(input, &index);
        tokens = tokens->next;
    }

    ASTNode* ast = build_AST(&(sentinel->next));
    int result = interpret_ast(ast);
    return result;
}

void run_default(char* input) {
    if (input == NULL) {
        input = "6 - 3 * 2";
    }

    Token* tokens = calloc(1, sizeof(Token));
    Token* sentinel = tokens;
    size_t index = 0;
    while (input[index] != '\0') {
        tokens->next = next_token(input, &index);
        tokens = tokens->next;
    }

    for (Token* token = sentinel->next; token; token = token->next) {
        print_token(token);
        printf("\n");
    }

    printf("\n");

    ASTNode* ast = build_AST(&(sentinel->next));
    print_AST(ast);
    printf("\n\n");

    int result = interpret_ast(ast);
    printf("%s = %d\n", input, result);
}

void print_usage() {
    fprintf(stderr, "Usage:\n");
    fprintf(stderr, "  ./main <optional: input>: run input or default\n");
    fprintf(stderr, "  ./main test run : run tests\n");
    fprintf(stderr, "  ./main test save : save expected results\n");
    exit(1);
}


int main(int argc, char** argv) {
    // TODO: operator precedence
    // TODO: float
    // TODO: functions (sqrt, ...)
    // TODO: use automaton to tokenize

    if (argc == 1) {
        run_default(NULL);
    }
    else if (argc == 2) {
        run_default(argv[1]);
    } else if (argc == 3) {
        if (strcmp(argv[1], "test") != 0) {
            fprintf(stderr, "Unknown argument: %s\n", argv[1]);
            print_usage();
        }

        if (strcmp(argv[2], "run") == 0) {
            tests_run();
        } else if (strcmp(argv[2], "save") == 0) {
            tests_save();
        } else {
            fprintf(stderr, "Unknown argument: %s\n", argv[2]);
            print_usage();
        }
    } else {
        fprintf(stderr, "Wrong number of arguments\n");
        print_usage();
    }

    return 0;
}
