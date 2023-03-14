#define _DEFAULT_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <dirent.h>
#include <string.h>
#include <errno.h>

#include "./ast.h"
#include "./token.h"

#define SV_IMPLEMENTATION
#include "./sv.h"

static Result evaluate_input(char* input) {
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

char* read_file(const char* filepath) {
    FILE* f = fopen(filepath, "r");
    if (f == NULL) {
        fprintf(stderr, "Could not open file %s: %s", filepath, strerror(errno));
        exit(1);
    }
    fseek(f, 0, SEEK_END);
    long fsize = ftell(f);
    rewind(f);

    char* content = malloc(fsize + 1);
    fread(content, fsize, 1, f);
    fclose(f);
    content[fsize] = '\0';
    return content;
}

String_View next_input(String_View* inputs) {
    String_View line = sv_chop_by_delim(inputs, '\n');
    String_View input = sv_trim(sv_chop_by_delim(&line, '#'));

    return input;
}

void save_test(const char* filepath, const char* filename, const char* dirpath) {
    char results_path[strlen("output/") + strlen(dirpath) + strlen(filename) + strlen("_output")];
    results_path[0] = '\0';
    strcat(results_path, dirpath);
    strcat(results_path, "output/");
    strcat(results_path, filename);
    strcat(results_path, "_output");
    FILE* results = fopen(results_path, "w+");
    if (results == NULL) {
        fprintf(stderr, "Could not open file %s: %s", results_path, strerror(errno));
        exit(1);
    }

    char* inputs = read_file(filepath);
    String_View sv_inputs = sv_from_cstr(inputs);

    String_View sv_input;
    while (sv_inputs.count > 0) {
        sv_input = next_input(&sv_inputs);
        if (sv_input.count == 0) {
            continue;
        }

        char input[sv_input.count + 1];
        sprintf(input, SV_Fmt, SV_Arg(sv_input));

        Result result = evaluate_input(input);
        if (result.type == RESULT_INT) {
            fprintf(results, "%d\n", result.vali);
            printf("%s = %d\n", input, result.vali);
        } else {
            fprintf(results, "%f\n", result.valf);
            printf("%s = %f\n", input, result.valf);
        }
    }
    free(inputs);
}


void run_test(const char* filepath, const char* filename, const char* dirpath) {
    char results_path[strlen("output/") + strlen(dirpath) + strlen(filename) + strlen("_output")];
    results_path[0] = '\0';
    strcat(results_path, dirpath);
    strcat(results_path, "output/");
    strcat(results_path, filename);
    strcat(results_path, "_output");

    char* inputs = read_file(filepath);
    String_View sv_inputs = sv_from_cstr(inputs);

    char* results = read_file(results_path);
    String_View sv_outputs = sv_from_cstr(results);

    String_View sv_input;
    String_View sv_expected;
    bool passed = true;
    while (sv_inputs.count > 0 && sv_outputs.count > 0) {
        sv_input = next_input(&sv_inputs);
        if (sv_input.count == 0) {
            continue;
        }
        sv_expected = sv_chop_by_delim(&sv_outputs, '\n');

        char input[sv_input.count + 1];
        sprintf(input, SV_Fmt, SV_Arg(sv_input));

        Result result = evaluate_input(input);
        char str_result[512]; // should be enough for everyone
        if (result.type == RESULT_INT) {
            sprintf(str_result, "%d", result.vali);
        } else {
            sprintf(str_result, "%f", result.valf);
        }

        char str_expected[sv_expected.count + 1];
        sprintf(str_expected, SV_Fmt, SV_Arg(sv_expected));
        if (strcmp(str_result, str_expected)) {
            printf("[ERROR] '%s' got '%s' expected '%s'\n", input, str_result, str_expected);
            passed = false;
        } else {
            // printf("[PASSED] %s = %s\n", input, str_result);
        }
    }
    free(inputs);

    if (sv_inputs.count > 0 || sv_outputs.count > 0) {
        printf("[ERROR] Different number of tests and results\n");
    }

    if (passed) {
        printf("[PASSED] All tests passed for %s\n", filepath);
    } else {
        printf("[ERROR] At least one test failed for %s\n", filepath);
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
