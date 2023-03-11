#ifndef AST_OPS_H_
#define AST_OPS_H_
#include <stddef.h>
#include "./ast.h"

ASTNode* ast_add(ASTNode* a, ASTNode* b);
ASTNode* ast_sub(ASTNode* a, ASTNode* b);
ASTNode* ast_mul(ASTNode* a, ASTNode* b);
ASTNode* ast_div(ASTNode* a, ASTNode* b);
void ast_neg(ASTNode* node);


#endif // AST_OPS_H_
