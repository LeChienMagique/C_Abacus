#ifndef AST_OPS_H_
#define AST_OPS_H_
#include <stddef.h>
#include "./ast.h"

Result ast_add(Result a, Result b);
Result ast_sub(Result a, Result b);
Result ast_mul(Result a, Result b);
Result ast_div(Result a, Result b);
Result ast_exp(Result a, Result b);
void ast_neg(ASTNode* node);
Result create_result_from_node(ASTNode* node);

#endif // AST_OPS_H_
