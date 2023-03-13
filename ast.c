#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <err.h>
#include <assert.h>
#include <string.h>
#include "./ast.h"
#include "./ast_operations.h"
/*
program = expr

expr = [unary] operand {operator operand}

operand = number
        | '(' expr ')'
        | symbol'(' expr {',' expr} ')'

operator = + | - | * | / | ^ | % | ==
*/
ASTNode* ast_next_expr(Token** tokens);
ASTNode* ast_next_operator(Token** tokens);
ASTNode* ast_next_operand(Token** tokens);
ASTNode* ast_next_number(Token** tokens);
void dump_tokens(Token** tokens);

void advance_tokens(Token** tokens) {
    Token* next = (*tokens)->next;
    TokenType type = (*tokens)->type;
    if (type == TOKEN_OPARENTHESIS || type == TOKEN_CPARENTHESIS) {
        if ((*tokens)->value) {
            free((*tokens)->value);
        }
        free(*tokens);
    }
    // if ((*tokens)->value) {
    //     free((*tokens)->value);
    // }
    // free(*tokens);
    *tokens = next;
}


ASTNode* create_node(Token* token, int type) {
    ASTNode* node = calloc(1, sizeof(ASTNode));
    node->token = token;
    node->type = type;
    return node;
}

void append_child(ASTNode* node, ASTNode* child) {
    ASTNode* last_child = node->children;
    if (last_child == NULL) {
        node->children = child;
        return;
    }
    for (; last_child->next; last_child = last_child->next) {}
    last_child->next = child;
}


int get_function_arity(ASTNode* func) {
    char* func_name = (char*) func->token->value;
    if (func_name == NULL) {
        assert(false && "unreachable");
    }

    if (strcmp(func_name, "sqrt") == 0) {
        return 1;
    }
    else if (strcmp(func_name, "facto") == 0) {
        return 1;
    }
    else if (strcmp(func_name, "fibo") == 0) {
        return 1;
    }
    else if (strcmp(func_name, "min") == 0) {
        return 2;
    }
    else if (strcmp(func_name, "max") == 0) {
        return 2;
    }
    else if (strcmp(func_name, "isprime") == 0) {
        return 1;
    }
    else if (strcmp(func_name, "gcd") == 0) {
        return 2;
    }
    else {
        printf("[ERROR] Arity not implemented for: ");
        print_node(func);
        printf("\n");
        assert(false);
    }

}


bool ast_is_operator(ASTNode* node) {
    switch (node->type) {
        case NODE_UMINUS:
        case NODE_UPLUS:
        case NODE_MINUS:
        case NODE_PLUS:
        case NODE_DIV:
        case NODE_EXP:
        case NODE_MOD:
        case NODE_EQUALITY:
        case NODE_ASSIGN:
        case NODE_MULT: {
            return true;
        }
        case NODE_FUNCTION:
        case NODE_EXPR:
        case NODE_SYMBOL:
        case NODE_INT:
        case NODE_PROGRAM:
        case NODE_FLOAT: {
            return false;
        }
    }
    assert(false && "unreachable");
}

OpArity get_operator_arity(ASTNode* optor) {
    switch (optor->type) {
        case NODE_MINUS: {
            return AR_MINUS;
        }
        case NODE_PLUS: {
            return AR_PLUS;
        }
        case NODE_MULT: {
            return AR_MULT;
        }
        case NODE_DIV: {
            return AR_DIV;
        }
        case NODE_EXP: {
            return AR_EXP;
        }
        case NODE_MOD: {
            return AR_MOD;
        }
        case NODE_EQUALITY: {
            return AR_EQUALITY;
        }
        case NODE_ASSIGN: {
            return AR_ASSIGN;
        }
        default: {
            printf("[ERROR] operator arity not implemented for: ");
            print_node(optor);
            printf("\n");
            assert(false);
        }
    }
}

OpPrecedence get_operator_precedence(ASTNode* optor) {
    switch (optor->type) {
        case NODE_MINUS: {
            return OP_MINUS;
        }
        case NODE_PLUS: {
            return OP_PLUS;
        }
        case NODE_MULT: {
            return OP_MULT;
        }
        case NODE_DIV: {
            return OP_DIV;
        }
        case NODE_MOD: {
            return OP_MOD;
        }
        case NODE_EXP: {
            return OP_EXP;
        }
        case NODE_EQUALITY: {
            return OP_EQUALITY;
        }
        case NODE_UPLUS: {
            return OP_PLUS;
        }
        case NODE_UMINUS: {
            return OP_UMINUS;
        }
        case NODE_ASSIGN: {
            return OP_ASSIGN;
        }
        default: {
            printf("[ERROR] operator precedence not implemented for: ");
            print_node(optor);
            printf("\n");
            assert(false);
        }
    }
}

ASTNode* ast_next_number(Token** tokens) {
    if (*tokens == NULL) {
        return NULL;
    }

    ASTNode* number = create_node(*tokens, -1);
    switch ((*tokens)->type) {
        case TOKEN_INT: {
            number->type = NODE_INT;
            int* value = malloc(sizeof(int));
            *value = atoi(number->token->value);
            number->value = (void*) value;
        } break;
        case TOKEN_FLOAT: {
            number->type = NODE_FLOAT;
            double* value = malloc(sizeof(double));
            *value = atof(number->token->value);
            number->value = (void*) value;
        } break;
        default: {
            free(number);
            return NULL;
        }
    }

    advance_tokens(tokens);
    return number;
}


ASTNode* ast_next_operand(Token** tokens) {
    //operand = number
    //        | ( expr )
    //        | symbol'(' expr {',' expr} ')'
    //        | symbol
    if (*tokens == NULL) {
        return NULL;
    }

    // number
    ASTNode* op = ast_next_number(tokens);
    if (op) {
        return op;
    }

    // symbol'(' expr {',' expr} ')'
    if ((*tokens)->type == TOKEN_SYMBOL) {
        ASTNode* symbol = create_node(*tokens, -1);
        advance_tokens(tokens);

        // function call
        if (*tokens != NULL && (*tokens)->type == TOKEN_OPARENTHESIS) {
            symbol->type = NODE_FUNCTION;
            advance_tokens(tokens);

            ASTNode* expr = ast_next_expr(tokens);
            if (expr) {
                append_child(symbol, expr);
            }

            while (*tokens != NULL && (*tokens)->type == TOKEN_COMMA) {
                advance_tokens(tokens);
                expr = ast_next_expr(tokens);
                if (expr == NULL) {
                    printf("[ERROR] Expected expression but got: ");
                    print_token(*tokens);
                    printf("\n");
                    assert(false);
                }
                append_child(symbol, expr);
            }

            if (*tokens == NULL || (*tokens)->type != TOKEN_CPARENTHESIS) {
                printf("[ERROR] Mismatched parenthesis\n");
                assert(false);
            }
            advance_tokens(tokens);

        }
        // variable
        else {
            symbol->type = NODE_SYMBOL;
        }
        return symbol;
    }

    if ((*tokens)->type == TOKEN_OPARENTHESIS) {
        advance_tokens(tokens);

        op = ast_next_expr(tokens);

        if (*tokens == NULL || (*tokens)->type != TOKEN_CPARENTHESIS) {
            printf("[ERROR] Mismatched parenthesis\n");
            dump_tokens(tokens);
            printf("\n");
            assert(false);
        }

        advance_tokens(tokens);
        return op;
    }

    return NULL;
}

ASTNode* ast_next_operator(Token** tokens) {
    if (*tokens == NULL) {
        return NULL;
    }

    ASTNode* optor = create_node(*tokens, -1);
    switch ((*tokens)->type) {
        case TOKEN_PLUS: {
            optor->type = NODE_PLUS;
        } break;
        case TOKEN_MINUS: {
            optor->type = NODE_MINUS;
        } break;
        case TOKEN_MULT: {
            optor->type = NODE_MULT;
        } break;
        case TOKEN_DIV: {
            optor->type = NODE_DIV;
        } break;
        case TOKEN_EXP: {
            optor->type = NODE_EXP;
        } break;
        case TOKEN_MOD: {
            optor->type = NODE_MOD;
        } break;
        case TOKEN_EQUALITY: {
            optor->type = NODE_EQUALITY;
        } break;
        case TOKEN_ASSIGN: {
            optor->type = NODE_ASSIGN;
        } break;
        default: {
            free(optor);
            return NULL;
        }
    }
    size_t token_len = strlen((char*) (*tokens)->value);
    void* value = calloc(token_len + 1, sizeof(char));
    memcpy(value, (*tokens)->value, token_len);
    optor->value = value;
    advance_tokens(tokens);
    return optor;
}

ASTNode* ast_next_unary(Token** tokens) {
    ASTNode* unary;
    switch ((*tokens)->type) {
        case TOKEN_PLUS: {
            unary = create_node(*tokens, NODE_UPLUS);
        } break;
        case TOKEN_MINUS: {
            unary = create_node(*tokens, NODE_UMINUS);
        } break;
        default: {
            return NULL;
        }
    }
    size_t token_len = strlen((char*) (*tokens)->value);
    void* value = calloc(token_len + 1, sizeof(char));
    memcpy(value, (*tokens)->value, token_len);
    unary->value = value;
    advance_tokens(tokens);
    return unary;
}

void ast_add_operator(ASTNode* optor, ASTNode* root) {
    if (ast_is_operator(root->children)) {
        // root->children replace with last operator
        if (get_operator_precedence(root->children) < get_operator_precedence(optor)) {
            ASTNode* before_last_operand = root->children->children;
            for (unsigned int i = 0; i < (get_operator_arity(root->children) - 2); i++) {
                before_last_operand = before_last_operand->next;
            }
            optor->children = before_last_operand->next;
            before_last_operand->next = NULL;
            append_child(root->children, optor);
        }
        else {
            optor->children = root->children;
            root->children = NULL;
            append_child(root, optor);
        }
    }
    else {
        optor->children = root->children;
        root->children = NULL;
        append_child(root, optor);
    }
}

ASTNode* ast_next_expr(Token** tokens) {
    if (*tokens == NULL) {
        return NULL;
    }
    // expr = [+ | -] operand {(operator operand) | ("(" operand ")")}
    ASTNode* expr = create_node(NULL, NODE_EXPR);

    // [+ | -]
    ASTNode* unary = ast_next_unary(tokens);

    // operand
    ASTNode* operand = ast_next_operand(tokens);
    if (operand == NULL) {
        return NULL;
    }

    if (unary) {
        append_child(unary, operand);
        append_child(expr, unary);
    } else {
        append_child(expr, operand);
    }

    // {operator operand}
    ASTNode* optor = ast_next_operator(tokens);
    if (optor != NULL) {
        while (optor != NULL) {
            ast_add_operator(optor, expr);

            operand = ast_next_operand(tokens);
            append_child(optor, operand);

            optor = ast_next_operator(tokens);
        }
    }

    // "(" operand ")"
    while (*tokens != NULL && (*tokens)->type == TOKEN_OPARENTHESIS) {
        optor = create_node(NULL, NODE_MULT);
        ast_add_operator(optor, expr);

        advance_tokens(tokens);

        operand = ast_next_operand(tokens);
        append_child(optor, operand);

        if ((*tokens)->type != TOKEN_CPARENTHESIS) {
            errx(EXIT_FAILURE, "Mismatched parenthesis");
        }
        advance_tokens(tokens);

    }

    return expr;
}

void dump_tokens(Token** tokens) {
    while (*tokens) {
        print_token(*tokens);
        printf(" | ");
        advance_tokens(tokens);
    }

}

ASTNode* build_AST(Token** tokens) {
    // node that parenthesis tokens will be freed during this process
    ASTNode* ast = calloc(1, sizeof(ASTNode));
    ast->type = NODE_PROGRAM;
    append_child(ast, ast_next_expr(tokens));

    while (*tokens && (*tokens)->type == TOKEN_SEMICOLON) {
        advance_tokens(tokens);
        append_child(ast, ast_next_expr(tokens));
    }

    if (*tokens) {
        printf("[ERROR] leftover tokens: ");
        dump_tokens(tokens);
        printf("\n");
        assert(false);
    }

    return ast;
}

Result* build_function_arguments(ASTNode* func, int* argc) {
    int arity = get_function_arity(func);

    int child_count = 0;
    for (ASTNode* child = func->children; child; child = child->next) {
        child_count++;
    }

    if (child_count != arity) {
        printf("[ERROR] Invalid number of arguments for function: ");
        print_node(func);
        printf("\n");
        exit(EXIT_FAILURE);
    }

    *argc = arity;

    Result* args = malloc(child_count * sizeof(Result));
    int i = 0;
    for (ASTNode* child = func->children; child; child = child->next) {
        args[i] = interpret_ast(child);
        i++;
    }
    return args;
}

// Variable linked list
typedef struct Variable {
    Result value;
    char* name;
    struct Variable* next;
} Variable;

Variable var_sentinel = {0};

Result* get_variable_value(const char* name) {
    Variable* var;
    for (var = var_sentinel.next; var; var = var->next) {
        if (strcmp(var->name, name) == 0) {
            return &var->value;
        }
    }
    return NULL;
}

void add_variable(ASTNode* var, Result value) {
    Variable* last;
    for (last = &var_sentinel; last->next; last = last->next) { }

    Variable* new_var = calloc(1, sizeof(Variable));
    new_var->value = value;

    size_t name_len = strlen(var->token->value);
    new_var->name = malloc(name_len + 1);
    new_var->name[name_len] = '\0';
    memcpy(new_var->name, var->token->value, name_len);

    last->next = new_var;
}

Result interpret_ast(ASTNode* node) {
    switch (node->type) {
        case NODE_PROGRAM: {
            ASTNode* expr;
            for (expr = node->children; expr->next; expr = expr->next) {
                interpret_ast(expr);
            }
            return interpret_ast(expr);
        }
        case NODE_UPLUS:
        case NODE_EXPR: {
            return interpret_ast(node->children);
        }
        case NODE_UMINUS: {
            return ast_neg(interpret_ast(node->children));
        }
        case NODE_PLUS: {
            Result a = interpret_ast(node->children);
            Result b = interpret_ast(node->children->next);
            Result result = ast_add(a, b);
            return result;
        }
        case NODE_MINUS: {
            Result a = interpret_ast(node->children);
            Result b = interpret_ast(node->children->next);
            Result result = ast_sub(a, b);
            return result;
        }
        case NODE_MULT: {
            Result a = interpret_ast(node->children);
            Result b = interpret_ast(node->children->next);
            Result result = ast_mul(a, b);
            return result;
        }
        case NODE_DIV: {
            Result a = interpret_ast(node->children);
            Result b = interpret_ast(node->children->next);
            Result result = ast_div(a, b);
            return result;
        }
        case NODE_EXP: {
            Result a = interpret_ast(node->children);
            Result b = interpret_ast(node->children->next);
            Result result = ast_exp(a, b);
            return result;
        }
        case NODE_MOD: {
            Result a = interpret_ast(node->children);
            Result b = interpret_ast(node->children->next);
            Result result = ast_mod(a, b);
            return result;
        }
        case NODE_EQUALITY: {
            Result a = interpret_ast(node->children);
            Result b = interpret_ast(node->children->next);
            Result result = ast_equal(a, b);
            return result;
        }
        case NODE_FLOAT:
        case NODE_INT: {
            return create_result_from_node(node);
        }
        case NODE_FUNCTION: {
            int argc;
            Result* argv = build_function_arguments(node, &argc);
            assert(node->token != NULL);
            Result result = ast_evaluate_function(node->token->value, argc, argv);
            free(argv);
            return result;
        }
        case NODE_SYMBOL: {
            Result* var_value = get_variable_value(node->token->value);
            if (var_value != NULL) {
                return *var_value;
            }
            errx(EXIT_FAILURE, "[ERROR] Undeclared variable");
        }
        case NODE_ASSIGN: {
            Result var_value = interpret_ast(node->children->next);
            add_variable(node->children, var_value);
            return var_value;
        }
        default: {
            printf("unimplemented node: ");
            print_node(node);
            printf("\n");
            assert(false);
        }
    }
}

void free_AST(ASTNode* root) {
    if (root->children != NULL) {
        ASTNode* next_child;
        for (ASTNode* child = root->children; child;) {
            next_child = child->next;
            free_AST(child);
            child = next_child;
        }
    }

    if (root->value != NULL) {
        free(root->value);
    }

    if (root->token) {
        if (root->token->value) {
            free(root->token->value);
        }
        free(root->token);
    }
    free(root);
}

void print_node(ASTNode* node) {
    switch (node->type) {
        case NODE_INT: {
            printf("NodeInt(%d)", *((int*) node->value));
        } break;
        case NODE_FLOAT: {
            printf("NodeFloat(%f)", *((double*) node->value));
        } break;
        case NODE_PLUS: {
            printf("NodePlus");
        } break;
        case NODE_MINUS: {
            printf("NodeMinus");
        } break;
        case NODE_UPLUS: {
            printf("NodeUnaryPlus");
        } break;
        case NODE_UMINUS: {
            printf("NodeUnaryMinus");
        } break;
        case NODE_EXPR: {
            printf("NodeExpr");
        } break;
        case NODE_PROGRAM: {
            printf("NodeProgram");
        } break;
        case NODE_MULT: {
            printf("NodeMult");
        } break;
        case NODE_DIV: {
            printf("NodeDiv");
        } break;
        case NODE_EXP: {
            printf("NodeExponent");
        } break;
        case NODE_MOD: {
            printf("NodeModulus");
        } break;
        case NODE_EQUALITY: {
            printf("NodeEquality");
        } break;
        case NODE_ASSIGN: {
            printf("NodeAssign");
        } break;
        case NODE_FUNCTION: {
            printf("NodeFunction(%s)", node->token->value);
        } break;
        case NODE_SYMBOL: {
            printf("NodeSymbol(%s)", node->token->value);
        } break;
    }
}

void print_AST(ASTNode* root) {
    print_node(root);
    if (root->children == NULL) {
        return;
    }
    printf(" -> {");
    for (ASTNode* child = root->children; child; child = child->next) {
        print_AST(child);
        if (child->next) {
            printf(", ");
        }
    }
    printf("}");
}
