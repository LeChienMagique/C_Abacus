#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <err.h>
#include <assert.h>
#include <stdbool.h>

#include "./token.h"
#include "./ast.h"
#include "./test.h"
void generate_dot(ASTNode* ast);

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
        input = "14 * (-2) + 3";
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

    generate_dot(ast);
}

void print_usage() {
    fprintf(stderr, "Usage:\n");
    fprintf(stderr, "  ./main <optional: input> : run input or default\n");
    fprintf(stderr, "  ./main test run : run tests\n");
    fprintf(stderr, "  ./main test save : save expected results\n");
    exit(1);
}

void write_node_label(FILE* f, ASTNode* node) {
    switch (node->type) {
        case NODE_INT: {
            fprintf(f, "[label=\"%d\"]\n", *((int*) node->value));
        } break;
        case NODE_MULT: {
            fprintf(f, "[label=\"*\"]\n");
        } break;
        case NODE_UPLUS:
        case NODE_PLUS: {
            fprintf(f, "[label=\"+\"]\n");
        } break;
        case NODE_DIV: {
            fprintf(f, "[label=\"/\"]\n");
        } break;
        case NODE_MINUS:
        case NODE_UMINUS: {
            fprintf(f, "[label=\"-\"]\n");
        } break;
        case NODE_EXPR: {
            fprintf(f, "[label=\"Expr\"]\n");
        } break;
        case NODE_TERM: {
            fprintf(f, "[label=\"Term\"]\n");
        } break;
    }
}

int _generate_dot(FILE* f, ASTNode* node, int parent, int nextid) {
    if (node == NULL) {
        return nextid;
    }
    int id = nextid++;

    fprintf(f, "\tnode%d", id);
    write_node_label(f, node);
    if (parent >= 0) {
        fprintf(f, "\tnode%d -- node%d\n", parent, id);
    }

    ASTNode* child = node->children;
    if (node->children) {
        for (; child->next; child = child->next) {
            nextid = _generate_dot(f, child, id, nextid);
        }
    }
    return _generate_dot(f, child, id, nextid);
}

void generate_dot(ASTNode* ast) {
    FILE* fp = fopen("graph.dot", "w+");
    if (fp == NULL) {
        err(1, "Could not open file 'graph.dot': ");
    }
    fprintf(fp, "graph {\n");
    _generate_dot(fp, ast, -1, 0);
    fprintf(fp, "}");
    fclose(fp);
    system("dot -Tsvg graph.dot > graph.svg");
}

int main(int argc, char** argv) {
    // TOOD: refactor code
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
