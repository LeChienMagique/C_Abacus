#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <err.h>
#include <assert.h>
#include <stdbool.h>

#include "./token.h"
#include "./ast.h"

int main() {
    // TODO: use automaton to tokenize
    char* input = "1 + 1";

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

    return 0;
}
