#define _XOPEN_SOURCE 700
#define _DEFAULT_SOURCE
#define _POSIX_C_SOURCE = 200809L
#include <stdio.h>
#include <stdlib.h>
#include <dirent.h>
#include <ftw.h>
#include <unistd.h>
#include <string.h>
#include <ctype.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/stat.h>

#include "src/ast.h"
#include "src/token.h"
#include "src/runtime.h"

#define UNUSED(x) (void)(x)
#define FLOAT_STR_LEN 64
#define INPUT_DELIM '~'

static int fail_count = 0;
static int pass_count = 0;
static int verbose_mode = 0;

typedef struct
{
    const char *filename;
    size_t line;

    char *input;
    int input_len;

    char *expected;
    int expected_len;
} Testcase;


const char *get_filename_ext(const char *filename)
{
    const char *dot = strrchr(filename, '.');
    if (!dot || dot == filename) return "";
    return dot + 1;
}

static void free_testcase(Testcase *test)
{
    free(test->input);
    free(test->expected);
    free(test);
}

static void shift_left_str(char *str)
{
    size_t i = 0;
    while (str[i] != '\0')
    {
        str[i - 1] = str[i];
        i++;
    }
    str[i - 1] = '\0';
}

static void trim_whitespaces(char *str)
{
    size_t i = 0;
    while (str[i] == ' ')
    {
        i++;
    }

    for (size_t j = 0; j < i; j++)
    {
        shift_left_str(str + i);
    }

    while (str[i] != '\0')
        i++;

    i--;

    while (str[i] == ' ')
    {
        i--;
    }
    str[i + 1] = '\0';
}

static Testcase *create_testcase(const char *filename, size_t line, FILE *f)
{
    Testcase *test = calloc(1, sizeof(Testcase));
    test->filename = filename;
    test->line = line;

    size_t input_len = 0;
    int input_red = getdelim(&(test->input), &input_len, INPUT_DELIM, f);
    test->input[input_red - 1] = '\0';
    trim_whitespaces(test->input);
    test->input_len = strlen(test->input);

    size_t expected_len = 0;
    int expected_red = getline(&(test->expected), &expected_len, f);
    if (expected_red > 0 && test->expected[expected_red - 1] == '\n')
        test->expected[expected_red - 1] = '\0';
    if (expected_red > 0)
        trim_whitespaces(test->expected);
    test->expected_len = strlen(test->expected);
    return test;
}

static int run_testcase(Testcase *test)
{
    Result result = evaluate_input(test->input);
    char *str_result = calloc(FLOAT_STR_LEN, 1);
    if (result.type == RESULT_INT)
    {
        snprintf(str_result, FLOAT_STR_LEN, "%d", result.vali);
    }
    else
    {
        snprintf(str_result, FLOAT_STR_LEN, "%.10f", result.valf);
    }

    char *pos = strstr(test->expected, str_result);
    if (pos == NULL)
    {
        fprintf(stderr, "%s:%zu:0: [FAIL] Expected: %s. Got: %s\n", test->filename, test->line, test->expected, str_result);
        fail_count++;
        free(str_result);
        return 0;
    }

    if (verbose_mode)
        printf("[PASSED] Input: %s. Expected: '%s'.\n", test->input, test->expected);

    pass_count++;
    free(str_result);
    return 1;
}

static int check_testcase_errors(Testcase *test)
{
    if (test->input_len == 0)
    {
        fprintf(stderr, "%s:%zu:0: [ERROR] Could not parse testcase. Input is empty.\n", test->filename, test->line);
        free_testcase(test);
        return 1;
    }
    else if (test->expected_len == -1)
    {
        fprintf(stderr, "%s:%zu:0: [ERROR] Could not parse testcase. Expected result is empty.\n", test->filename, test->line);
        free_testcase(test);
        return 1;
    }
    return 0;
}

static int test_file(const char *file)
{
    FILE *f = fopen(file, "r");

    size_t line = 1;
    while (!feof(f))
    {
        char c;
        if ((c = fgetc(f)) == '\n')
        {
            line++;
            continue;
        }
        else if (c == '#')
        {
            fscanf(f, "%*[^\n]\n");
            line++;
            continue;
        }

        ungetc(c, f);

        Testcase *test = create_testcase(file, line++, f);

        if (check_testcase_errors(test))
            continue;

        run_testcase(test);
        free_testcase(test);
    }

    fclose(f);
    return fail_count;
}

static int handle_tree_entry(const char *filepath, const struct stat *info,
                             const int typeflag, struct FTW *pathinfo)
{
    UNUSED(info);
    UNUSED(typeflag);
    UNUSED(pathinfo);
    if (strcmp(get_filename_ext(filepath), "test") != 0)
    {
        return 0;
    }
    test_file(filepath);
    return 0;
}

static int test_directory_tree(const char *const dirpath)
{
    if (dirpath == NULL || *dirpath == '\0')
        return -1;

    fail_count = 0;
    int result = nftw(dirpath, handle_tree_entry, 64, FTW_DEPTH);
    if (result != 0)
        return -1;

    return fail_count;
}

int main(int argc, char **argv)
{
    char c;
    while ((c = getopt (argc, argv, "v")) != -1)
    {
        switch (c)
        {
        case 'v':
            verbose_mode = 1;
            break;
        case '?':
            if (isprint(optopt))
                fprintf(stderr, "Unknown option `-%c'.\n", optopt);
            else
                fprintf(stderr, "Unknown option character `\\x%x'.\n", optopt);
            return 1;
        default:
            abort();
        }
    }

    test_directory_tree("tests");
    printf("\nNumber of tests passed: %d\n", pass_count);
    printf("Number of tests failed: %d\n", fail_count);
    return 0;
}
