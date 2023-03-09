#ifndef TOKEN_H
#define TOKEN_H

#define TOKEN_INT 1
#define TOKEN_PLUS 2
#define TOKEN_MINUS 3
#define TOKEN_MULT 4
#define TOKEN_PARENTHESIS 5

typedef struct Token {
    char* value;
    int type;
    struct Token* next;
} Token;

void print_token(Token* token);
Token* next_token(char* input, size_t* index);

#endif // TOKEN_H
