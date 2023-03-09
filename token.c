#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <assert.h>

#include "./token.h"

void print_type(int token_type) {
    switch (token_type) {
        case TOKEN_INT: {
            printf("TokenInt");
        } break;
        case TOKEN_PLUS: {
            printf("TokenPlus");
        } break;
        case TOKEN_MINUS: {
            printf("TokenMinus");
        } break;
        case TOKEN_PARENTHESIS: {
            printf("TokenParenthesis");
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

Token* token_next_int(char* input, size_t* index) {
    char c = input[*index];
    size_t start = *index;
    while (c != '\0' && is_digit(c)) {
        (*index)++;
        c = input[*index];
    }
    size_t end = *index;
    char* value = malloc(end - start);
    value = memcpy(value, input + start, end - start);
    Token* token = calloc(1, sizeof(Token));
    token->value = value;
    token->type = TOKEN_INT;

    return token;
}

Token* token_next_operator(char* input, size_t* index) {
    char c = input[*index];
    Token* token = calloc(1, sizeof(Token));
    switch (c) {
        case '+': {
            token->type = TOKEN_PLUS;
            token->value = "+";
        } break;
        case '-': {
            token->type = TOKEN_MINUS;
            token->value = "-";
        } break;
        default: {
            printf("%c operator not implemented\n", c);
            assert(false);
        }
    }
    (*index)++;
    return token;
}

Token* next_token(char* input, size_t* index) {
    char c = input[*index];
    while (c != '\0') {
        c = input[*index];
        if (c == ' ') {
            (*index)++;
        } else if (is_digit(c)) {
            return token_next_int(input, index);
        } else if (is_operator(c)) {
            return token_next_operator(input, index);
        } else if (c == '(' || c == ')') {
            Token* token = calloc(1, sizeof(Token));
            token->type = TOKEN_PARENTHESIS;
            token->value = c == ')' ? ")" : "(";
            (*index)++;
            return token;
        }
        else {
            printf("not implemented at index %lu\n", *index);
            assert(false);
        }

    }
    return NULL;
}
