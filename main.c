#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <err.h>
#include <assert.h>
#include <stdbool.h>

#include "./token.h"
#include "./ast.h"


int main() {
    // TODO: float
    // TODO: operator precedence
    // TODO: functions (sqrt, ...)

    // TODO: use automaton to tokenize
    char* input = "-2 * 4";

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
    printf("Input: %s | Result: %d\n", input, result);
    // assert(result == 5);

    return 0;
}
