#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <err.h>
#include <assert.h>
#include <stdbool.h>

#include "./token.h"
#include "./ast.h"
#include "./test.h"

int GENERATE_GRAPH = 0;
int DEBUG_MODE = 0;

void print_usage() {
    fprintf(stderr, "Usage:\n");
    fprintf(stderr, "  ./main <input> [options] : run input\n");
    fprintf(stderr, "  ./main test run [options] : run tests\n");
    fprintf(stderr, "  ./main test save : save expected results\n");
    fprintf(stderr, "Options:\n");
    fprintf(stderr, "  --debug               Print debug informations\n");
    fprintf(stderr, "  --graph               Generate AST graph\n");
    exit(1);
}

void write_node_label(FILE* f, ASTNode* node) {
    switch (node->type) {
        case NODE_INT: {
            fprintf(f, "[label=\"%d\"]\n", *((int*) node->value));
        } break;
        case NODE_FLOAT: {
            fprintf(f, "[label=\"%f\"]\n", *((double*) node->value));
        } break;
        case NODE_SYMBOL: {
            fprintf(f, "[label=\"%s\"]\n", node->token->value);
        } break;
        case NODE_FUNCTION: {
            fprintf(f, "[label=\"Func(%s)\"]\n", node->token->value);
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
        case NODE_EXP: {
            fprintf(f, "[label=\"^\"]\n");
        } break;
        case NODE_MOD: {
            fprintf(f, "[label=\"%%\"]\n");
        } break;
        case NODE_EQUALITY: {
            fprintf(f, "[label=\"==\"]\n");
        } break;
        case NODE_ASSIGN: {
            fprintf(f, "[label=\"=\"]\n");
        } break;
        case NODE_MINUS:
        case NODE_UMINUS: {
            fprintf(f, "[label=\"-\"]\n");
        } break;
        case NODE_EXPR: {
            fprintf(f, "[label=\"Expr\"]\n");
        } break;
        case NODE_PROGRAM: {
            fprintf(f, "[label=\"Program\"]\n");
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

Result evaluate_input(char* input) {
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
            print_token(token);
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

void run(char* input) {
    Result result = evaluate_input(input);

    if (result.type == RESULT_INT) {
        // printf("%s = %d\n", input, result.vali);
        printf("%d\n", result.vali);
    } else {
        // printf("%s = %f\n", input,  result.valf);
        printf("%f\n", result.valf);
    }
}

void repl_mode() {
    char input[1024]; // should be enough
    bool quit = false;

    while (!quit) {
        printf(">>> ");
        if (fgets(input, 1024, stdin)) {
            if (strcmp(input, "exit") == 0) {
                quit = true;
                break;
            }

            int len = strlen(input);
            if (len > 0) {
                if (len - 1 == 0) {
                    continue;
                }
                if (input[len - 1] < 40) {
                    input[len - 1] = '\0';
                }
            }

            Result result = evaluate_input(input);
            if (result.type == RESULT_INT) {
                printf("> %d\n", result.vali);
            } else {
                printf("> %f\n", result.valf);
            }
        }
    }
}


int main(int argc, char** argv) {
    // TODO: REPL mode
    // TODO: ideas:
    //       - add some math functions
    // TODO: beautify debug graph
    // TODO: use automaton to tokenize
    /*
    Usage:
    ./main <input> [options]: run input
    ./main test run : run tests
    ./main test save : save expected results
    Options:
    --graph  Generate AST graph
    --debug  Prints debug information
    */
    if (argc >= 2) {
        // test
        if (strcmp(argv[1], "test") == 0) {
            if (argc != 3) {
                fprintf(stderr, "Wrong number of arguments\n");
                print_usage();
            }
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
        }
        else if (strcmp(argv[1], "--repl") == 0) {
            repl_mode();
            exit(0);
        }
        // run user input
        else {
            char* input = argv[1];
            for (int i = 2; i < argc; i++) {
                if (strcmp(argv[i], "--debug") == 0) {
                    DEBUG_MODE = 1;
                } else if (strcmp(argv[i], "--graph") == 0) {
                    GENERATE_GRAPH = 1;
                } else {
                    fprintf(stderr, "Unknown argument: %s\n", argv[i]);
                    print_usage();
                }
            }
            run(input);
        }
    } else {
        fprintf(stderr, "Not enough arguments\n");
        print_usage();
    }

    return 0;
}
