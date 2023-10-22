#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <assert.h>
#include <stdbool.h>

#include "./src/token.h"
#include "./src/ast.h"
#include "./src/runtime.h"

void print_usage() {
    fprintf(stderr, "Usage:\n");
    fprintf(stderr, "  ./main <input> [options]          Run input\n");
    fprintf(stderr, "  ./main --repl                     Run in REPL mode\n");
    // fprintf(stderr, "  ./main test run                   Run tests\n");
    // fprintf(stderr, "  ./main test save                  Save expected results\n");
    fprintf(stderr, "Options:\n");
    fprintf(stderr, "  --debug                           Print debug informations\n");
    fprintf(stderr, "  --graph                           Generate AST graph\n");
    exit(1);
}

void repl_mode() {
    // TODO: use getline instead of hardcoded buffer
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

                // not sure if this is portable
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
    // TODO: refactor code (partially done)
    // TODO: implement scopes (partially done)
    // TODO: implement function definition
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
        // if (strcmp(argv[1], "test") == 0) {
        //     if (argc != 3) {
        //         fprintf(stderr, "Wrong number of arguments\n");
        //         print_usage();
        //     }
        //     if (strcmp(argv[1], "test") != 0) {
        //         fprintf(stderr, "Unknown argument: %s\n", argv[1]);
        //         print_usage();
        //     }
        //     if (strcmp(argv[2], "run") == 0) {
        //         tests_run();
        //     } else if (strcmp(argv[2], "save") == 0) {
        //         tests_save();
        //     } else {
        //         fprintf(stderr, "Unknown argument: %s\n", argv[2]);
        //         print_usage();
        //     }
        // }
        if (strcmp(argv[1], "--repl") == 0) {
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
