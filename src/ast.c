#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <errno.h>
#include <assert.h>
#include <string.h>
#include "./ast.h"
#include "./ast_operations.h"

const OpPrecedence OPERATOR_PRECEDENCE[NODE_COUNT + 1] = {-1, -1, OP_UPLUS, OP_UMINUS, OP_PLUS, OP_MINUS, OP_DIV, OP_MULT, OP_EXP, OP_MOD, OP_EQUALITY, OP_ASSIGN, -1, -1, -1, -1, -1, -1};

const bool IS_OPERATOR[NODE_COUNT + 1] = {false, false, true, true, true, true, true, true, true, true, true, true, false, false, false, false, false, false, false};

const char* NODE_NAMES[NODE_COUNT + 1] = {"NodeInt", "NodeFloat", "NodeUplus", "NodeUminus", "NodePlus", "NodeMinus", "NodeDiv", "NodeMult", "NodeExp", "NodeMod", "NodeEquality", "NodeAssign", "NodeBuiltinFunction", "NodeFunction", "NodeSymbol", "NodeExpr", "NodeProgram", "NodeFuncdef", "!NodeCount!"};

const NodeType NODE_TYPES[TOKEN_COUNT + 1] = {
    NODE_INT, NODE_FLOAT, NODE_PLUS, NODE_MINUS, NODE_MULT, NODE_DIV, NODE_EXP, NODE_MOD, NODE_EQUALITY, NODE_ASSIGN, NODE_COUNT, NODE_COUNT, NODE_SYMBOL, NODE_COUNT, NODE_COUNT, NODE_COUNT
};

const OpArity OPERATOR_ARITY[NODE_COUNT + 1] = {-1, -1, AR_UMINUS, AR_UPLUS, AR_PLUS, AR_MINUS, AR_DIV, AR_MULT, AR_EXP, AR_MOD, AR_EQUALITY, AR_ASSIGN, -1, -1, -1, -1, -1};

// Function linked list with sentinel
typedef struct Function {
    char* name;
    size_t arity;
    ASTNode* body;
    ASTNode* args;
    struct EvalScope* scope;
    struct Function* next;
} Function;

// Variable linked list with sentinel
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

funcdef = 'def' symbol '(' symbol {, symbol} ')' = expr

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
ASTNode* ast_next_funcdef(Token** tokens);
void dump_tokens(Token** tokens);
Function* get_function(EvalScope* scope, const char* name);
void free_scope(EvalScope* scope);

void dump_scope(EvalScope* scope) {
    printf("Functions: ");
    for (Function* func = scope->functions->next; func; func = func->next) {
        printf("%s, ", func->name);
    }
    printf("\nVariables: ");
    for (Variable* var = scope->variables->next; var; var = var->next) {
        printf("%s, ", var->name);
    }
    printf("\n");
}

size_t ast_count_children(ASTNode* node) {
    size_t count = 0;
    for (ASTNode* child = node->children; child; child = child->next) {
        count++;
    }
    return count;
}

bool check_token_type(Token* token, TokenType expected) {
    return token != NULL && token->type == expected;
}

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


void advance_free_tokens(Token** tokens) {
    Token* next = (*tokens)->next;
    if ((*tokens)->value) {
        free((*tokens)->value);
    }
    free(*tokens);
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


int get_bultin_function_arity(ASTNode* func) {
    char* func_name = (char*) func->token->value;
    if (func_name == NULL) {
        fprintf(stderr, "[ERROR] func_name = NULL");
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

int get_function_arity(EvalScope* scope, ASTNode* func_node) {
    if (is_builtin_function(func_node->token->value)) {
        return get_bultin_function_arity(func_node);
    }

    Function* func = get_function(scope, func_node->token->value);
    return func->arity;
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

    // symbol'(' {expr {',' expr}} ')'
    if ((*tokens)->type == TOKEN_SYMBOL) {
        ASTNode* symbol = create_node(*tokens, -1);
        advance_tokens(tokens);

        // function call
        if (check_token_type(*tokens, TOKEN_OPARENTHESIS)) {
            if (is_builtin_function(symbol->token->value)) {
                symbol->type = NODE_BUILTIN_FUNCTION;
            } else {
                symbol->type = NODE_FUNCTION;
            }
            advance_tokens(tokens);

            ASTNode* expr;
            while (!check_token_type(*tokens, TOKEN_CPARENTHESIS)) {
                expr = ast_next_expr(tokens);
                append_child(symbol, expr);

                if (check_token_type(*tokens, TOKEN_COMMA)) {
                    advance_tokens(tokens);    // skip comma
                }
            }
            advance_tokens(tokens); // skip CPAR
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


ASTNode* ast_next_funcdef(Token** tokens) {
    // funcdef = 'def' symbol '(' {symbol {, symbol}} ')' = expr

    // create func def node
    // create func node
    // retrieve func args
    // parse func definition
    // funcdef node -> {{func node}, {func definition}}
    if (!check_token_type(*tokens, TOKEN_FUNCDEF)) {
        return NULL;
    }
    ASTNode* funcdef = create_node(*tokens, NODE_FUNCDEF);
    advance_tokens(tokens);

    if (!check_token_type(*tokens, TOKEN_SYMBOL)) {
        fprintf(stderr, "Expected function name but found: ");
        print_token(stderr, *tokens);
        exit(1);
    }
    ASTNode* func = create_node(*tokens, NODE_FUNCTION);
    append_child(funcdef, func);
    advance_tokens(tokens);

    if (!check_token_type(*tokens, TOKEN_OPARENTHESIS)) {
        fprintf(stderr, "Expected open parenthesis after function declaration but found: ");
        print_token(stderr, *tokens);
        exit(1);
    }
    advance_tokens(tokens);

    // parse func args
    while (!check_token_type(*tokens, TOKEN_CPARENTHESIS)) {
        if (!check_token_type(*tokens, TOKEN_SYMBOL)) {
            fprintf(stderr, "Expected symbol but found: ");
            print_token(stderr, *tokens);
            exit(1);
        }

        ASTNode* arg = create_node(*tokens, NODE_SYMBOL);
        append_child(func, arg);
        advance_tokens(tokens); // skip symbol

        if (check_token_type(*tokens, TOKEN_COMMA)) {
            advance_tokens(tokens);    // skip comma
        }
    }
    advance_tokens(tokens);

    if (!check_token_type(*tokens, TOKEN_ASSIGN)) {
        fprintf(stderr, "Expected '=' after function declaration but found: ");
        print_token(stderr, *tokens);
        exit(1);
    }
    advance_free_tokens(tokens);

    // parse func body
    ASTNode* body = ast_next_expr(tokens);
    if (body == NULL) {
        fprintf(stderr, "Expected function body but found nothing.");
        exit(1);
    }
    append_child(funcdef, body);

    return funcdef;
}


void dump_tokens(Token** tokens) {
    while (*tokens) {
        print_token(stdout, *tokens);
        printf(" | ");
        advance_tokens(tokens);
    }

}

ASTNode* build_AST(Token** tokens) {
    // parenthesis tokens will be freed during this process
    ASTNode* ast = calloc(1, sizeof(ASTNode));
    ast->type = NODE_PROGRAM;

    ASTNode* node = ast_next_funcdef(tokens);
    if (node == NULL) {
        node = ast_next_expr(tokens);
    }
    append_child(ast, node);

    while (check_token_type(*tokens, TOKEN_SEMICOLON)) {
        advance_free_tokens(tokens);
        ASTNode* node = ast_next_funcdef(tokens);
        if (node == NULL) {
            node = ast_next_expr(tokens);
        }
        append_child(ast, node);
    }

    if (*tokens) {
        printf("[ERROR] Leftover tokens: ");
        dump_tokens(tokens);
        printf("\n");
        exit(1);
    }

    return ast;
}

Result* build_function_arguments(EvalScope* scope, ASTNode* func) {
    int arity = get_function_arity(scope, func);

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

    Result* args = malloc(child_count * sizeof(Result));
    int i = 0;
    for (ASTNode* child = func->children; child; child = child->next) {
        args[i] = interpret_ast(child);
        i++;
    }
    return args;
}

EvalScope* create_scope(EvalScope* parent) {
    EvalScope* scope = calloc(1, sizeof(EvalScope));
    scope->functions = calloc(1, sizeof(Function));
    scope->variables = calloc(1, sizeof(Variable));
    scope->parent = parent;
    return scope;
}

Variable* get_variable(EvalScope* scope, const char* name) {
    Variable* var = scope->variables;
    for (var = var->next; var; var = var->next) {
        if (strcmp(var->name, name) == 0) {
            return var;
        }
    }
    return NULL;
}

void set_variable_value(EvalScope* scope, const char* name, Result value) {
    Variable* existing = get_variable(scope, name);
    if (existing != NULL) {
        existing->value = value;
        return;
    }

    Variable* last = scope->variables;
    for (; last->next; last = last->next) { }

    Variable* new_var = calloc(1, sizeof(Variable));
    new_var->value = value;

    size_t name_len = strlen(name);
    new_var->name = malloc(name_len + 1);
    new_var->name[name_len] = '\0';
    memcpy(new_var->name, name, name_len);

    last->next = new_var;
}

Function* get_function(EvalScope* scope, const char* name) {
    for (Function* func = scope->functions->next; func; func = func->next) {
        if (strcmp(func->name, name) == 0) {
            return func;
        }
    }
    return NULL;
}

void free_function_fields(Function* func) {
    free(func->name);
    free_scope(func->scope);
}

void redefine_function(Function* old, EvalScope* scope, ASTNode* funcdef_node, int arity) {
    free_scope(old->scope);
    old->arity = arity;
    old->scope = create_scope(scope);
    old->args = funcdef_node->children->children;
    old->body = funcdef_node->children->next;
}

Function* add_function(EvalScope* scope, ASTNode* funcdef_node, int arity) {
    Function* existing = get_function(scope, funcdef_node->children->token->value);
    if (existing != NULL) {
        redefine_function(existing, scope, funcdef_node, arity);
        return existing;
    }

    Function* new_func = calloc(1, sizeof(Function));
    Function* last = scope->functions;
    for (; last->next; last = last->next) { }

    ASTNode* func_node = funcdef_node->children;

    size_t name_len = strlen(func_node->token->value);
    new_func->name = malloc(name_len + 1);
    new_func->name[name_len] = '\0';
    memcpy(new_func->name, func_node->token->value, name_len);

    new_func->arity = arity;
    new_func->scope = create_scope(scope);
    new_func->args = funcdef_node->children->children;
    new_func->body = funcdef_node->children->next;

    last->next = new_func;

    return new_func;
}

Result _interpret_ast(EvalScope* scope, ASTNode* node);

Result interpret_ast(ASTNode* node) {
    EvalScope* top_scope = create_scope(NULL);

    Result res = _interpret_ast(top_scope, node);
    free_scope(top_scope);
    return res;
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
    case NODE_BUILTIN_FUNCTION: {
        if (!is_builtin_function(node->token->value)) {
            fprintf(stderr, "Unknown function: '%s'\n", node->token->value);
            exit(20);
        }
        Result* argv = build_function_arguments(scope, node);
        assert(node->token != NULL);
        Result result = ast_evaluate_builtin_function(
                            node->token->value,
                            get_function_arity(scope, node),
                            argv);
        free(argv);
        return result;
    }
    case NODE_SYMBOL: {
        Variable* var =  get_variable(scope, node->token->value);
        Result* var_value = &var->value;
        if (var_value != NULL) {
            return *var_value;
        }
        fprintf(stderr, "[ERROR] Undeclared variable: %s", node->token->value);
        exit(1);
    }
    case NODE_ASSIGN: {
        if (node->children->type != NODE_SYMBOL) {
            fprintf(stderr, "[ERROR] Cannot assign value to a literal");
            exit(1);
        }
        Result var_value = _interpret_ast(scope, node->children->next);
        set_variable_value(scope, node->children->token->value, var_value);
        return var_value;
    }
    case NODE_FUNCDEF: {
        ASTNode* func_node = node->children;
        int arity = 0;
        for (ASTNode* arg = func_node->children; arg; arg = arg->next) {
            arity++;
        }
        add_function(scope, node, arity);
        return (Result) {
            .type = RESULT_INT,
            .vali = 0,
            .valf = 0
        };
    }

    case NODE_FUNCTION: {
        Function* func = get_function(scope, node->token->value);
        ASTNode* arg_name = func->args;
        ASTNode* arg_value = node->children; // func->{args}
        size_t passed_args_count = ast_count_children(node);
        if (passed_args_count != func->arity) {
            fprintf(stderr, "Invalid number of arguments for function: %s. Expected %lu but got %lu",
                    func->name,
                    func->arity, passed_args_count);
            exit(1);
        }

        for (size_t i = 0; i < func->arity; i++) {
            set_variable_value(func->scope, arg_name->token->value, _interpret_ast(scope, arg_value));
            arg_name = arg_name->next;
            arg_value = arg_value->next;
        }

        return _interpret_ast(func->scope, func->body);
    }
    default: {
        printf("[ERROR] Unimplemented node: ");
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

void free_scope(EvalScope* scope);
void free_variables(Variable* variables);

void free_functions(Function* functions) {
    if (functions == NULL) {
        return;
    }

    Function* func = functions;
    if (func->scope->functions == NULL) {
        printf("func '%s', scope is null\n", func->name);
    }
    free_scope(func->scope);
    free(func->name);
    free_functions(functions->next);
    free(func);
}

void free_variables(Variable* variables) {
    if (variables == NULL) {
        return;
    }

    Variable* var = variables;
    free(var->name);
    free_variables(variables->next);
    free(var);
}

void free_scope(EvalScope* scope) {
    free_functions(scope->functions->next);
    free_variables(scope->variables->next);
    free(scope->functions);
    free(scope->variables);
    free(scope);
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
