#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <assert.h>
#include <err.h>

#include "./token.h"

void print_type(int token_type) {
    switch (token_type) {
        case TOKEN_INT: {
            printf("TokenInt");
        } break;
        case TOKEN_FLOAT: {
            printf("TokenFloat");
        } break;
        case TOKEN_PLUS: {
            printf("TokenPlus");
        } break;
        case TOKEN_MINUS: {
            printf("TokenMinus");
        } break;
        case TOKEN_OPARENTHESIS: {
            printf("TokenOpenParenthesis");
        } break;
        case TOKEN_CPARENTHESIS: {
            printf("TokenCloseParenthesis");
        } break;
        case TOKEN_MULT: {
            printf("TokenMult");
        } break;
        case TOKEN_DIV: {
            printf("TokenDiv");
        } break;
        default: {
            printf("UnknownToken");
        }
    }
}

void print_token(Token* token) {
    print_type(token->type);
    printf(" value: '%s'", token->value);
}

bool is_digit(char c) {
    return c >= '0' && c <= '9';
}

bool is_letter(char c) {
    return (c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z');
}

bool is_alpha(char c) {
    return is_digit(c) || is_letter(c);
}

bool is_operator(char c) {
    return c == '+' || c == '-' || c == '*' || c == '/';
}

Token* create_token(TokenType type, const char* start, size_t length) {
    Token* token = calloc(1, sizeof(Token));
    char* value = malloc((length + 1) * sizeof(char));
    value = memcpy(value, start, length);
    value[length] = '\0';
    token->value = value;
    token->type = type;
    return token;
}

Token* token_next_number(const char* input, size_t* index) {
    char c = input[*index];
    size_t start = *index;
    while (c != '\0' && is_digit(c)) {
        c = input[++(*index)];
    }

    bool is_float = false;
    if (c != '\0' && c == '.') {
        is_float = true;
        c = input[++(*index)];
        while (c != '\0' && is_digit(c)) {
            c = input[++(*index)];
        }
    }

    size_t end = *index;
    return create_token(is_float ? TOKEN_FLOAT : TOKEN_INT, input + start, end - start);
}

Token* token_next_operator(const char* input, size_t* index) {
    char c = input[*index];
    Token* token;
    switch (c) {
        case '+': {
            token = create_token(TOKEN_PLUS, input + (*index)++, 1);
        } break;
        case '-': {
            token = create_token(TOKEN_MINUS, input + (*index)++, 1);
        } break;
        case '*': {
            token = create_token(TOKEN_MULT, input + (*index)++, 1);
        } break;
        case '/': {
            token = create_token(TOKEN_DIV, input + (*index)++, 1);
        } break;
        default: {
            printf("[ERROR] %c operator not implemented\n", c);
            assert(false);
        }
    }
    return token;
}

Token* next_token(const char* input, size_t* index) {
    char c = input[*index];
    while (c != '\0') {
        if (c == ' ') {
            (*index)++;
        } else if (is_digit(c)) {
            return token_next_number(input, index);
        } else if (is_operator(c)) {
            return token_next_operator(input, index);
        } else if (c == '(') {
            return create_token(TOKEN_OPARENTHESIS, input + (*index)++, 1);
        } else if (c == ')') {
            return create_token(TOKEN_CPARENTHESIS, input + (*index)++, 1);
        }
        else {
            printf("[ERROR] unknown token starting with char: %d at index %lu\n", (int) input[*index], *index);
            printf("%s\n", input);
            for (size_t i = 0; i < *index; i++) {
                printf(" ");
            }
            printf("^\n");
            assert(false);
        }
        c = input[*index];

    }
    return NULL;
}
