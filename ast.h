#ifndef AST_H
#define AST_H
#include "./token.h"

#define NODE_INT 1
#define NODE_PLUS 2
#define NODE_MINUS 3
#define NODE_EXPR 4
#define NODE_TERM 5

typedef struct ASTNode {
    Token* token;
    void* value;
    int type;
    struct ASTNode* children;
    struct ASTNode* next;
} ASTNode;

ASTNode* ast_next_expr(Token** tokens);
ASTNode* ast_next_operator(Token** tokens);
ASTNode* ast_next_term(Token** tokens);
ASTNode* ast_next_operand(Token** tokens);
ASTNode* ast_next_number(Token** tokens);

void print_node(ASTNode* node);
void print_AST(ASTNode* root);

ASTNode* build_AST(Token** tokens);
#endif // AST_H
