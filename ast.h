#ifndef AST_H
#define AST_H
#include "./token.h"
#include <stdbool.h>

typedef enum {
    NODE_INT,
    NODE_PLUS,
    NODE_UPLUS,
    NODE_MINUS,
    NODE_UMINUS,
    NODE_DIV,
    NODE_MULT,
    NODE_EXPR,
    NODE_TERM,
} NodeType;

typedef enum {
    OP_PLUS = 11,
    OP_MINUS = 11,
    OP_DIV = 12,
    OP_MULT = 12,
} OpPrecedence;

typedef enum {
    AR_PLUS = 2,
    AR_MINUS = 2,
    AR_DIV = 2,
    AR_MULT = 2,
} OpArity;


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
OpPrecedence get_operator_precedence(ASTNode* optor);
OpArity get_operator_arity(ASTNode* optor);

bool ast_is_operator(ASTNode* node);
void print_node(ASTNode* node);
void print_AST(ASTNode* root);

ASTNode* build_AST(Token** tokens);
int interpret_ast(ASTNode* node);
#endif // AST_H
