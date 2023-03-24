#ifndef AST_H
#define AST_H
#include "./token.h"
#include <stdbool.h>

typedef enum {
    NODE_INT = 0,
    NODE_FLOAT,
    // unary operators
    NODE_UPLUS,
    NODE_UMINUS,
    // binary operators
    NODE_PLUS,
    NODE_MINUS,
    NODE_DIV,
    NODE_MULT,
    NODE_EXP,
    NODE_MOD,
    NODE_EQUALITY,
    NODE_ASSIGN,
    //
    NODE_FUNCTION,
    NODE_SYMBOL,
    NODE_EXPR,
    NODE_PROGRAM,
    NODE_COUNT
} NodeType;

typedef enum {
    OP_ASSIGN = 2,
    OP_EQUALITY = 8,
    OP_PLUS = 11,
    OP_MINUS = 11,
    OP_DIV = 12,
    OP_MULT = 12,
    OP_MOD = 12,
    OP_EXP = 13,
    OP_UMINUS = 14,
    OP_UPLUS = 14,
} OpPrecedence;

typedef enum {
    AR_UMINUS = 1,
    AR_UPLUS = 1,
    AR_PLUS = 2,
    AR_MINUS = 2,
    AR_DIV = 2,
    AR_MULT = 2,
    AR_EXP = 2,
    AR_MOD = 2,
    AR_EQUALITY = 2,
    AR_ASSIGN = 2,
} OpArity;


typedef struct ASTNode {
    Token* token;
    void* value;
    NodeType type;
    struct ASTNode* children;
    struct ASTNode* next;
} ASTNode;

typedef enum  {
    RESULT_INT,
    RESULT_FLOAT
} ResultType;

typedef struct {
    ResultType type;
    int vali;
    double valf;
} Result;





// ASTNode* ast_next_expr(Token** tokens);
// ASTNode* ast_next_operator(Token** tokens);
// ASTNode* ast_next_term(Token** tokens);
// ASTNode* ast_next_operand(Token** tokens);
// ASTNode* ast_next_number(Token** tokens);
// OpPrecedence get_operator_precedence(ASTNode* optor);
// OpArity get_operator_arity(ASTNode* optor);

bool ast_is_operator(ASTNode* node);
void print_node(ASTNode* node);
void print_AST(ASTNode* root);

ASTNode* build_AST(Token** tokens);
Result interpret_ast(ASTNode* node);

void free_AST(ASTNode* ast);
#endif // AST_H
