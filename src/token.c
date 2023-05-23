#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <assert.h>

#include "./token.h"

void print_type(FILE* out, int token_type) {
    switch (token_type) {
    case TOKEN_INT: {
        fprintf(out, "TokenInt");
    }
    break;
    case TOKEN_FLOAT: {
        fprintf(out, "TokenFloat");
    }
    break;
    case TOKEN_PLUS: {
        fprintf(out, "TokenPlus");
    }
    break;
    case TOKEN_MINUS: {
        fprintf(out, "TokenMinus");
    }
    break;
    case TOKEN_OPARENTHESIS: {
        fprintf(out, "TokenOpenParenthesis");
    }
    break;
    case TOKEN_CPARENTHESIS: {
        fprintf(out, "TokenCloseParenthesis");
    }
    break;
    case TOKEN_MULT: {
        fprintf(out, "TokenMult");
    }
    break;
    case TOKEN_DIV: {
        fprintf(out, "TokenDiv");
    }
    break;
    case TOKEN_EXP: {
        fprintf(out, "TokenExponent");
    }
    break;
    case TOKEN_MOD: {
        fprintf(out, "TokenModulus");
    }
    break;
    case TOKEN_EQUALITY: {
        fprintf(out, "TokenEquality");
    }
    break;
    case TOKEN_ASSIGN: {
        fprintf(out, "TokenAssign");
    }
    break;
    case TOKEN_FUNCDEF: {
        fprintf(out, "TokenFuncdef");
    }
    break;
    case TOKEN_SYMBOL: {
        fprintf(out, "TokenSymbol");
    }
    break;
    case TOKEN_SEMICOLON: {
        fprintf(out, "TokenSemicolon");
    }
    break;
    }
}

void print_token(FILE* out, Token* token) {
    if (token == NULL) {
        fprintf(out, "nothing");
        return;
    }
    print_type(out, token->type);
    fprintf(out, " value: '%s'", token->value);
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

#define OPERATORS_COUNT 9
typedef struct {
    const char* value;
    const size_t len;
    const TokenType type;
} Operator;

// CAREFUL: if an operator is a prefix of another operator it should be placed after because of how
//          ast_next_operator works
const Operator operators[OPERATORS_COUNT] = {{"+", 1, TOKEN_PLUS}, {"-", 1, TOKEN_MINUS}, {"*", 1, TOKEN_MULT},
    {"/", 1, TOKEN_DIV}, {"^", 1, TOKEN_EXP}, {"%", 1, TOKEN_MOD}, {"==", 2, TOKEN_EQUALITY}, {"=", 1, TOKEN_ASSIGN},
    {"def", 3, TOKEN_FUNCDEF}
};
// const char operators[OPERATORS_COUNT] = {'+', '-', '*', '/', '^', '%', '='};
bool is_operator(const char* input, size_t index) {
    bool result;
    for (size_t i = 0; i < OPERATORS_COUNT; i++) {
        result = true;
        for (size_t j = 0; j < operators[i].len; j++) {
            if (input[index + j] == '\0') {
                return false;
            }
            if (input[index + j] != operators[i].value[j]) {
                result = false;
                break;
            }
        }

        if (result) {
            return true;
        }
    }
    return false;
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
    bool is_op;
    size_t start = *index;
    for (size_t i = 0; i < OPERATORS_COUNT; i++) {
        is_op = true;
        Operator op = operators[i];
        for (size_t j = 0; j < op.len; j++) {
            if (input[*index + j] == '\0') {
                return NULL;
            }
            if (input[*index + j] != op.value[j]) {
                is_op = false;
                break;
            }
        }

        if (is_op) {
            *index += op.len;
            return create_token(op.type, input + start, op.len);
        }
    }
    return NULL;
}

Token* token_next_symbol(const char* input, size_t* index) {
    char c = input[*index];
    size_t start = *index;
    // TODO: allow symbols to have alphanumeric chars
    while (c != '\0' && is_letter(c)) {
        c = input[++(*index)];
    }
    size_t end = *index;
    return create_token(TOKEN_SYMBOL, input + start, end - start);
}

Token* next_token(const char* input, size_t* index) {
    char c = input[*index];
    Token* tok;
    while (c != '\0') {
        if (c == ' ') {
            (*index)++;
        } else if (is_digit(c)) {
            return token_next_number(input, index);
        } else if ((tok = token_next_operator(input, index)) != NULL) {
            return tok;
        } else if (c == '(') {
            return create_token(TOKEN_OPARENTHESIS, input + (*index)++, 1);
        } else if (c == ')') {
            return create_token(TOKEN_CPARENTHESIS, input + (*index)++, 1);
        } else if (is_letter(c)) {
            return token_next_symbol(input, index);
        } else if (c == ',') {
            return create_token(TOKEN_COMMA, input + (*index)++, 1);
        } else if (c == ';') {
            return create_token(TOKEN_SEMICOLON, input + (*index)++, 1);
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

void free_tokens(Token* tokens) {
    if (tokens->next) {
        free_tokens(tokens->next);
    }
    if (tokens->value) {
        free(tokens->value);
    }
    free(tokens);
}
