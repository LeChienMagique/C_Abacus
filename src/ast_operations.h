#ifndef AST_OPS_H_
#define AST_OPS_H_
#include <stddef.h>
#include "./ast.h"

Result ast_add(Result a, Result b);
Result ast_sub(Result a, Result b);
Result ast_mul(Result a, Result b);
Result ast_div(Result a, Result b);
Result ast_exp(Result a, Result b);
Result ast_mod(Result a, Result b);
Result ast_equal(Result a, Result b);
Result ast_evaluate_builtin_function(const char* func_name, int argc, Result* argv);
Result ast_neg(Result x);
Result create_result_from_node(ASTNode* node);

#endif // AST_OPS_H_
