#ifndef AST_H
#define AST_H
#include "./token.h"

typedef enum {
    NODE_INT,
    NODE_PLUS,
    NODE_UPLUS,
    NODE_MINUS,
    NODE_UMINUS,
    NODE_MULT,
    NODE_EXPR,
    NODE_TERM,
} NodeType;

typedef struct ASTNode {
    Token* token;
    void* value;
    NodeType type;
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
int interpret_ast(ASTNode* node);
#endif // AST_H
