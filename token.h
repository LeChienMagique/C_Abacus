#ifndef TOKEN_H
#define TOKEN_H


typedef enum {
    TOKEN_INT,
    TOKEN_PLUS,
    TOKEN_MINUS,
    TOKEN_MULT,
    TOKEN_DIV,
    TOKEN_OPARENTHESIS,
    TOKEN_CPARENTHESIS,
} TokenType;

typedef struct Token {
    char* value;
    TokenType type;
    struct Token* next;
} Token;

void print_token(Token* token);
Token* next_token(char* input, size_t* index);

#endif // TOKEN_H
