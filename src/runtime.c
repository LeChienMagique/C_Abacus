#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <errno.h>

#include "runtime.h"

int GENERATE_GRAPH = 0;
int DEBUG_MODE = 0;

static const char* NODE_FMT[NODE_COUNT + 1] = {"INT", "FLOAT", "+", "-", "+", "-", "/", "*", "^", "%", "==", "=", "FUNCDEF", "FUNC", "SYMBOL", "Expr", "Program", "!NodeCount!"};

static void write_node_label(FILE* f, ASTNode* node) {
    switch (node->type) {
    case NODE_INT: {
        fprintf(f, "[label=\"%d\"]\n", *((int*) node->value));
    }
    break;
    case NODE_FLOAT: {
        fprintf(f, "[label=\"%f\"]\n", *((double*) node->value));
    }
    break;
    case NODE_SYMBOL: {
        fprintf(f, "[label=\"%s\"]\n", node->token->value);
    }
    break;
    case NODE_FUNCTION: {
        fprintf(f, "[label=\"Func(%s)\"]\n", node->token->value);
    }
    break;
    case NODE_FUNCDEF: {
        fprintf(f, "[label=\"FuncDef\"]\n");
    }
    break;
    default: {
        fprintf(f, "[label=\"%s\"]", NODE_FMT[node->type]);
    }
    }
}

static int _generate_dot(FILE* f, ASTNode* node, int parent, int nextid) {
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

static void generate_dot(ASTNode* ast) {
    FILE* fp = fopen("graph.dot", "w+");
    if (fp == NULL) {
        fprintf(stderr, "Could not open file 'graph.dot': %s", strerror(errno));
        exit(1);
    }
    fprintf(fp, "graph {\n");
    // fprintf(fp, "bgcolor=\"grey\"\n");
    _generate_dot(fp, ast, -1, 0);
    fprintf(fp, "}");
    fclose(fp);
    system("dot -Tsvg graph.dot > graph.svg");
}

Result evaluate_input(const char* input) {
    Token* tokens = calloc(1, sizeof(Token));
    Token* sentinel = tokens;
    size_t index = 0;
    while (index < strlen(input)) {
        tokens->next = next_token(input, &index);
        tokens = tokens->next;
    }
    if (DEBUG_MODE) {
        printf("Tokens:\n");
        for (Token* token = sentinel->next; token; token = token->next) {
            print_token(stdout, token);
            printf("\n");
        }
        printf("\n");
    }

    ASTNode* ast = build_AST(&(sentinel->next));

    if (DEBUG_MODE) {
        print_AST(ast);
        printf("\n\n");
    }
    if (GENERATE_GRAPH) {
        generate_dot(ast);
    }

    Result result = interpret_ast(ast);

    // tokens are actually freed during free_AST
    free_AST(ast);
    free(sentinel);
    return result;
}

void run(const char* input) {
    Result result = evaluate_input(input);

    if (result.type == RESULT_INT) {
        // printf("%s = %d\n", input, result.vali);
        printf("%d\n", result.vali);
    } else {
        // printf("%s = %f\n", input,  result.valf);
        printf("%.10f\n", result.valf);
    }
}
