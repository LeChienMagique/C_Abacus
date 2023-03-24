#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <errno.h>
#include <assert.h>
#include <string.h>
#include "./ast.h"
#include "./ast_operations.h"

const OpPrecedence OPERATOR_PRECEDENCE[NODE_COUNT + 1] = {-1, -1, OP_UPLUS, OP_UMINUS, OP_PLUS, OP_MINUS, OP_DIV, OP_MULT, OP_EXP, OP_MOD, OP_EQUALITY, OP_ASSIGN, -1, -1, -1, -1, -1};

const bool IS_OPERATOR[NODE_COUNT + 1] = {false, false, true, true, true, true, true, true, true, true, true, true, false, false, false, false, false};

const char* NODE_NAMES[NODE_COUNT + 1] = {"NodeInt", "NodeFloat", "NodeUplus", "NodeUminus", "NodePlus", "NodeMinus", "NodeDiv", "NodeMult", "NodeExp", "NodeMod", "NodeEquality", "NodeAssign", "NodeFunction", "NodeSymbol", "NodeExpr", "NodeProgram", "!NodeCount!"};

const NodeType NODE_TYPES[TOKEN_COUNT + 1] = {
    NODE_INT, NODE_FLOAT, NODE_PLUS, NODE_MINUS, NODE_MULT, NODE_DIV, NODE_EXP, NODE_MOD, NODE_EQUALITY, NODE_ASSIGN, NODE_COUNT, NODE_COUNT, NODE_SYMBOL, NODE_COUNT, NODE_COUNT, NODE_COUNT
};

const OpArity OPERATOR_ARITY[NODE_COUNT + 1] = {-1, -1, AR_UMINUS, AR_UPLUS, AR_PLUS, AR_MINUS, AR_DIV, AR_MULT, AR_EXP, AR_MOD, AR_EQUALITY, AR_ASSIGN, -1, -1, -1, -1, -1};

// Function linked list
typedef struct Function {
    char* name;
    int arity;
    struct EvalScope* scope;
    struct Function* next;
} Function;

// Variable linked list
typedef struct Variable {
    Result value;
    char* name;
    struct Variable* next;
} Variable;


typedef struct EvalScope {
    Variable* variables;
    Function* functions;
    struct EvalScope* parent;
} EvalScope;

/*
program = funcdef | expr

funcdef = 'def' name '(' arg {, arg} ')' = exprb

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

#define BUILTIN_FUNC_COUNT 7
const char* BUILTIN_FUNCS[BUILTIN_FUNC_COUNT] = {"sqrt", "facto", "fibo", "min", "max", "isprime", "gcd"};
bool is_builtin_function(const char* func_name) {
    for (int i = 0; i < BUILTIN_FUNC_COUNT; i++) {
        if (strcmp(BUILTIN_FUNCS[i], func_name) == 0) {
            return true;
        }
    }
    return false;
}


int get_function_arity(ASTNode* func) {
    char* func_name = (char*) func->token->value;
    if (func_name == NULL) {
        fprintf(stderr, "[ERROR] func_name == NULL");
        exit(1);
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
        // should print to stderr
        printf("[ERROR] Arity not implemented for: ");
        print_node(func);
        printf("\n");
        exit(1);
    }

}

bool ast_is_operator(ASTNode* node) {
    return IS_OPERATOR[node->type];
}

OpArity get_operator_arity(ASTNode* optor) {
    OpArity ar = OPERATOR_ARITY[optor->type];
    if ((int) ar == -1) {
        printf("[ERROR] Operator arity not implemented for: ");
        print_node(optor);
        printf("\n");
        exit(1);
    }
    return ar;
}

OpPrecedence get_operator_precedence(ASTNode* optor) {
    OpPrecedence prec = OPERATOR_PRECEDENCE[optor->type];
    if ((int) prec == -1) {
        printf("[ERROR] Operator precedence not implemented for: ");
        print_node(optor);
        printf("\n");
        exit(1);
    }
    return prec;
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
    }
    break;
    case TOKEN_FLOAT: {
        number->type = NODE_FLOAT;
        double* value = malloc(sizeof(double));
        *value = atof(number->token->value);
        number->value = (void*) value;
    }
    break;
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
            advance_tokens(tokens);
            symbol->type = NODE_FUNCTION;

            int arity = get_function_arity(symbol);
            ASTNode* expr;
            for (int i = 0; i < arity; i++) {
                expr = ast_next_expr(tokens);
                if (expr == NULL) {
                    fprintf(stderr, "[ERROR] %s function accepts %d arguments, but only got %d.\n",
                            symbol->token->value, arity, i);
                    exit(1);
                }
                append_child(symbol, expr);

                if (i != arity - 1) {
                    if ((*tokens)->type != TOKEN_COMMA) {
                        fprintf(stderr, "[ERROR] Expected comma after expression.");
                        exit(1);
                    }
                    advance_tokens(tokens); // skip comma
                }
            }

            if (*tokens == NULL || (*tokens)->type != TOKEN_CPARENTHESIS) {
                fprintf(stderr, "[ERROR] Mismatched parenthesis\n");
                exit(1);
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
            exit(1);
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

    NodeType node_type = NODE_TYPES[(*tokens)->type];
    if (!IS_OPERATOR[node_type]) {
        return NULL;
    }

    ASTNode* optor = create_node(*tokens, node_type);

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
    }
    break;
    case TOKEN_MINUS: {
        unary = create_node(*tokens, NODE_UMINUS);
    }
    break;
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
            for (int i = 0; i < (int) (get_operator_arity(root->children) - 2); i++) {
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
            fprintf(stderr, "Mismatched parenthesis");
            exit(1);
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
        exit(1);
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

Result* get_variable_value(EvalScope* scope, const char* name) {
    Variable* var = scope->variables;
    for (var = var->next; var; var = var->next) {
        if (strcmp(var->name, name) == 0) {
            return &var->value;
        }
    }
    return NULL;
}

void add_variable(EvalScope* scope, ASTNode* var, Result value) {
    Variable* last;
    for (last = scope->variables; last->next; last = last->next) { }

    Variable* new_var = calloc(1, sizeof(Variable));
    new_var->value = value;

    size_t name_len = strlen(var->token->value);
    new_var->name = malloc(name_len + 1);
    new_var->name[name_len] = '\0';
    memcpy(new_var->name, var->token->value, name_len);

    last->next = new_var;
}

Function* add_function(EvalScope* scope, const char* name, int arity) {
    Function* last;
    for (last = scope->functions; last->next; last = last->next) { }

    Function* new_func = calloc(1, sizeof(Function));

    size_t name_len = strlen(name);
    new_func->name = malloc(name_len + 1);
    new_func->name[name_len] = '\0';
    memcpy(new_func->name, name, name_len);

    new_func->arity = arity;
    new_func->scope = calloc(1, sizeof(EvalScope));
    new_func->scope->variables = calloc(1, sizeof(Variable));

    last->next = new_func;
    return new_func;
}

Result _interpret_ast(EvalScope* scope, ASTNode* node);

Result interpret_ast(ASTNode* node) {
    EvalScope* top_scope = calloc(1, sizeof(EvalScope));
    top_scope->functions = calloc(1, sizeof(Function));
    top_scope->variables = calloc(1, sizeof(Variable));

    return _interpret_ast(top_scope, node);
}

Result _interpret_ast(EvalScope* scope, ASTNode* node) {
    switch (node->type) {
    case NODE_PROGRAM: {
        ASTNode* expr;
        for (expr = node->children; expr->next; expr = expr->next) {
            _interpret_ast(scope, expr);
        }
        return _interpret_ast(scope, expr);
    }
    case NODE_UPLUS:
    case NODE_EXPR: {
        return _interpret_ast(scope, node->children);
    }
    case NODE_UMINUS: {
        return ast_neg(_interpret_ast(scope, node->children));
    }
    case NODE_PLUS: {
        Result a = _interpret_ast(scope, node->children);
        Result b = _interpret_ast(scope, node->children->next);
        Result result = ast_add(a, b);
        return result;
    }
    case NODE_MINUS: {
        Result a = _interpret_ast(scope, node->children);
        Result b = _interpret_ast(scope, node->children->next);
        Result result = ast_sub(a, b);
        return result;
    }
    case NODE_MULT: {
        Result a = _interpret_ast(scope, node->children);
        Result b = _interpret_ast(scope, node->children->next);
        Result result = ast_mul(a, b);
        return result;
    }
    case NODE_DIV: {
        Result a = _interpret_ast(scope, node->children);
        Result b = _interpret_ast(scope, node->children->next);
        Result result = ast_div(a, b);
        return result;
    }
    case NODE_EXP: {
        Result a = _interpret_ast(scope, node->children);
        Result b = _interpret_ast(scope, node->children->next);
        Result result = ast_exp(a, b);
        return result;
    }
    case NODE_MOD: {
        Result a = _interpret_ast(scope, node->children);
        Result b = _interpret_ast(scope, node->children->next);
        Result result = ast_mod(a, b);
        return result;
    }
    case NODE_EQUALITY: {
        Result a = _interpret_ast(scope, node->children);
        Result b = _interpret_ast(scope, node->children->next);
        Result result = ast_equal(a, b);
        return result;
    }
    case NODE_FLOAT:
    case NODE_INT: {
        return create_result_from_node(node);
    }
    case NODE_FUNCTION: {
        if (!is_builtin_function(node->token->value)) {
            fprintf(stderr, "func name: '%s'\n", node->token->value);
            exit(20);
        }
        int argc;
        Result* argv = build_function_arguments(node, &argc);
        assert(node->token != NULL);
        Result result = ast_evaluate_function(node->token->value, argc, argv);
        free(argv);
        return result;
    }
    case NODE_SYMBOL: {
        Result* var_value = get_variable_value(scope, node->token->value);
        if (var_value != NULL) {
            return *var_value;
        }
        fprintf(stderr, "[ERROR] Undeclared function");
        exit(1);
    }
    case NODE_ASSIGN: {
        if (node->children->type != NODE_SYMBOL) {
            fprintf(stderr, "[ERROR] Cannot assign value to a literal");
            exit(1);
        }
        Result var_value = _interpret_ast(scope, node->children->next);
        add_variable(scope, node->children, var_value);
        return var_value;
    }
    default: {
        printf("unimplemented node: ");
        print_node(node);
        printf("\n");
        exit(1);
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
    const char* node_name = NODE_NAMES[node->type];
    switch (node->type) {
    case NODE_INT: {
        printf("%s(%d)", node_name, *((int*) node->value));
    }
    break;
    case NODE_FLOAT: {
        printf("%s(%f)", node_name, *((double*) node->value));
    }
    break;
    case NODE_FUNCTION: {
        printf("%s(%s)", node_name, node->token->value);
    }
    break;
    case NODE_SYMBOL: {
        printf("%s(%s)", node_name, node->token->value);
    }
    break;
    default: {
        printf("%s", node_name);
    }
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
