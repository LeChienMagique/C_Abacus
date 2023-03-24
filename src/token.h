#ifndef TOKEN_H
#define TOKEN_H


typedef enum {
    TOKEN_INT = 0,
    TOKEN_FLOAT,
    // operators
    TOKEN_PLUS,
    TOKEN_MINUS,
    TOKEN_MULT,
    TOKEN_DIV,
    TOKEN_EXP,
    TOKEN_MOD,
    TOKEN_EQUALITY,
    TOKEN_ASSIGN,
    //
    TOKEN_OPARENTHESIS,
    TOKEN_CPARENTHESIS,
    TOKEN_SYMBOL,
    TOKEN_COMMA,
    TOKEN_SEMICOLON,
    TOKEN_COUNT
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
