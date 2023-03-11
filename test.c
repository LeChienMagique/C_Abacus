#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <dirent.h>
#include <string.h>
#include <err.h>

#include "./ast.h"
#include "./token.h"

static ASTNode* evaluate_input(char* input) {
    Token* tokens = calloc(1, sizeof(Token));
    Token* sentinel = tokens;
    size_t index = 0;
    while (index < strlen(input)) {
        tokens->next = next_token(input, &index);
        tokens = tokens->next;
    }

    ASTNode* ast = build_AST(&(sentinel->next));
    return interpret_ast(ast);
}

void save_test(const char* filepath, const char* filename, const char* dirpath) {
    FILE* infile = fopen(filepath, "r");
    if (infile == NULL) {
        err(1, "Could not open file %s: ", filepath);
    }

    char results_path[strlen("output/") + strlen(dirpath) + strlen(filename) + strlen("_output")];
    results_path[0] = '\0';
    strcat(results_path, dirpath);
    strcat(results_path, "output/");
    strcat(results_path, filename);
    strcat(results_path, "_output");
    FILE* results = fopen(results_path, "w+");
    if (results == NULL) {
        err(1, "Could not open file %s: ", results_path);
    }

    char* input;
    ssize_t nread;
    size_t n = 0;
    while ((nread = getline(&input, &n, infile)) != -1) {
        if (input[nread - 1] == '\n') {
            input[nread - 1] = '\0';
        }
        ASTNode* result = evaluate_input(input);
        if (result->type == NODE_INT) {
            fprintf(results, "%s = %d\n", input, *(int*) result->value);
            fprintf(stdout, "%s = %d\n", input, *(int*) result->value);
        } else {
            fprintf(results, "%s = %f\n", input, *(double*) result->value);
            fprintf(stdout, "%s = %f\n", input, *(double*) result->value);
        }
    }
}


void run_test(const char* filepath, const char* filename, const char* dirpath) {
    FILE* infile = fopen(filepath, "r");
    if (infile == NULL) {
        err(1, "Could not open file %s: ", filepath);
    }

    char results_path[strlen("output/") + strlen(dirpath) + strlen(filename) + strlen("_output")];
    results_path[0] = '\0';
    strcat(results_path, dirpath);
    strcat(results_path, "output/");
    strcat(results_path, filename);
    strcat(results_path, "_output");
    FILE* results = fopen(results_path, "r");
    if (results == NULL) {
        err(1, "Could not open file %s: ", results_path);
    }

    char* input;
    char* expected;
    ssize_t nread;
    size_t n = 0;
    while ((nread = getline(&input, &n, infile)) != -1) {
        if (input[nread - 1] == '\n') {
            input[nread - 1] = '\0';
        }
        ASTNode* result = evaluate_input(input);

        n = 0;
        nread = getline(&expected, &n, results);
        if (nread == -1) {
            errx(1, "Different number of tests and results for test: %s", filepath);
        }

        char str_result[512]; // should be enough for everyone
        if (result->type == NODE_INT) {
            sprintf(str_result, "%d", *(int*) result->value);
        } else {
            sprintf(str_result, "%f", *(double*) result->value);
        }

        if (expected[nread - 1] == '\n') {
            expected[nread - 1] = '\0';
        }
        if (strcmp(str_result, expected) != 0) {
            printf("[ERROR] '%s' got '%s' expected '%s'\n", input, str_result, expected);
        } else {
            printf("[PASSED] %s = %s\n", input, str_result);
        }
    }
}


void tests_run() {
    DIR* d;
    struct dirent* dir;
    d = opendir("./tests");
    char file_path[1024];
    if (d) {
        while ((dir = readdir(d)) != NULL) {
            if (dir->d_type == DT_REG) {
                file_path[0] = '\0';
                strcat(file_path, "./tests/");
                strcat(file_path, dir->d_name);
                printf("Running test %s...\n", dir->d_name);
                run_test(file_path, dir->d_name, "./tests/");
            }
        }
        closedir(d);
    }
}

void tests_save() {
    DIR* d;
    struct dirent* dir;
    d = opendir("./tests");
    char file_path[1024];
    if (d) {
        while ((dir = readdir(d)) != NULL) {
            if (dir->d_type == DT_REG) {
                file_path[0] = '\0';
                strcat(file_path, "./tests/");
                strcat(file_path, dir->d_name);
                printf("Saving output for test %s...\n", dir->d_name);
                save_test(file_path, dir->d_name, "./tests/");
            }
        }
        closedir(d);
    }
}
