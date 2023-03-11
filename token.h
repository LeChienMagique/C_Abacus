#ifndef TOKEN_H
#define TOKEN_H


typedef enum {
    TOKEN_INT,
    TOKEN_FLOAT,
    TOKEN_PLUS,
    TOKEN_MINUS,
    TOKEN_MULT,
    TOKEN_DIV,
    TOKEN_EXP,
    TOKEN_OPARENTHESIS,
    TOKEN_CPARENTHESIS,
} TokenType;

typedef struct Token {
    char* value;
    TokenType type;
    struct Token* next;
} Token;

void print_token(Token* token);
Token* next_token(const char* input, size_t* index);
void free_tokens(Token* tokens);
#endif // TOKEN_H
