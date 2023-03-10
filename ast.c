#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <err.h>
#include <assert.h>
#include "./ast.h"
/*
program = expr

expr = term {operator term}

term = [unary] operand {operator operand}

operand = number
        | '(' expr ')'

operator = + | - | * | /
*/
ASTNode* ast_next_expr(Token** tokens);
ASTNode* ast_next_operator(Token** tokens);
ASTNode* ast_next_term(Token** tokens);
ASTNode* ast_next_operand(Token** tokens);
ASTNode* ast_next_number(Token** tokens);


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


bool ast_is_operator(ASTNode* node) {
    switch (node->type) {
        case NODE_MINUS:
        case NODE_PLUS:
        case NODE_DIV:
        case NODE_MULT: {
            return true;
        }
        default: {
            return false;
        }
    }
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
        default: {
            free(number);
            return NULL;
        }
    }
    *tokens = (*tokens)->next;
    return number;
}


ASTNode* ast_next_operand(Token** tokens) {
    //operand = number
    //        | ( expr )
    if (*tokens == NULL) {
        return NULL;
    }

    ASTNode* op = ast_next_number(tokens);
    if (op) {
        return op;
    }

    if ((*tokens)->type != TOKEN_OPARENTHESIS) {
        printf("[ERROR] Expected parenthesis followed by expression but got: ");
        print_token(*tokens);
        printf("\n");
        assert(false);
    }
    *tokens = (*tokens)->next;

    op = ast_next_expr(tokens);

    if (*tokens == NULL || (*tokens)->type != TOKEN_CPARENTHESIS) {
        printf("[ERROR] Mismatched parenthesis\n");
        assert(false);
    }

    *tokens = (*tokens)->next;
    return op;
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
        default: {
            free(optor);
            return NULL;
        }
    }
    optor->value = (*tokens)->value;
    *tokens = (*tokens)->next;
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
    unary->value = (*tokens)->value;
    *tokens = (*tokens)->next;
    return unary;
}

ASTNode* ast_next_term(Token** tokens) {
    if (*tokens == NULL) {
        return NULL;
    }
    // term = [+ | -] operand {operator operand}
    ASTNode* term = create_node(NULL, NODE_TERM);

    // [+ | -]
    ASTNode* unary = ast_next_unary(tokens);

    // operand
    ASTNode* operand = ast_next_operand(tokens);
    if (operand == NULL) {
        printf("Expected operand got: ");
        print_token(*tokens);
        printf("\n");
        assert(false);
    }
    if (unary) {
        append_child(unary, operand);
        append_child(term, unary);
    } else {
        append_child(term, operand);
    }

    // {operator operand}
    ASTNode* optor = ast_next_operator(tokens);
    while (optor != NULL) {
        if (ast_is_operator(term->children)) {
            // term->children replace with last operator
            if (get_operator_precedence(term->children) < get_operator_precedence(optor)) {
                print_node(optor);
                printf("\n");
                ASTNode* before_last_operand = term->children->children;
                for (unsigned int i = 0; i < (get_operator_arity(term->children) - 2); i++) {
                    before_last_operand = before_last_operand->next;
                }
                optor->children = before_last_operand->next;
                before_last_operand->next = NULL;
                append_child(term->children, optor);
            }
            else {
                optor->children = term->children;
                term->children = NULL;
                append_child(term, optor);
            }
        }
        else {
            optor->children = term->children;
            term->children = NULL;
            append_child(term, optor);
        }

        operand = ast_next_operand(tokens);
        append_child(optor, operand);

        optor = ast_next_operator(tokens);
    }
    return term->children;
}

ASTNode* ast_next_expr(Token** tokens) {
    if (*tokens == NULL) {
        return NULL;
    }

    // expr = term {operator term}
    ASTNode* expr = create_node(NULL, NODE_EXPR);

    // term
    ASTNode* term = ast_next_term(tokens);
    if (term == NULL) {
        return NULL;
    }
    append_child(expr, term);

    // {operator term}
    ASTNode* optor = ast_next_operator(tokens);
    while (optor) {
        if (ast_is_operator(expr->children)) {
            // expr->children replace with last operator
            if (get_operator_precedence(expr->children) < get_operator_precedence(optor)) {
                ASTNode* before_last_operand = expr->children->children;
                for (unsigned int i = 0; i < (get_operator_arity(expr->children) - 2); i++) {
                    before_last_operand = before_last_operand->next;
                }
                optor->children = before_last_operand->next;
                before_last_operand->next = NULL;
                append_child(expr->children, optor);
                // append_child(expr, );
            } else {
                optor->children = expr->children;
                expr->children = NULL;
                append_child(expr, optor);
            }
        }
        else {
            optor->children = expr->children;
            expr->children = NULL;
            append_child(expr, optor);
        }

        term = ast_next_term(tokens);
        append_child(optor, term);

        optor = ast_next_operator(tokens);
    }
    return expr;
}

ASTNode* build_AST(Token** tokens) {
    ASTNode* ast = ast_next_expr(tokens);
    if (*tokens) {
        printf("[ERROR] leftover tokens: ");
        while (*tokens) {
            print_token(*tokens);
            printf(" | ");
            *tokens = (*tokens)->next;
        }
        printf("\n");
        assert(false);
    }
    return ast;
}

int interpret_ast(ASTNode* node) {
    switch (node->type) {
        case NODE_UPLUS:
        case NODE_EXPR:
        case NODE_TERM: {
            return interpret_ast(node->children);
        }
        case NODE_UMINUS: {
            return (-1) * interpret_ast(node->children);
        }
        case NODE_PLUS: {
            return interpret_ast(node->children) + interpret_ast(node->children->next);
        }
        case NODE_MINUS: {
            return interpret_ast(node->children) - interpret_ast(node->children->next);
        }
        case NODE_MULT: {
            return interpret_ast(node->children) * interpret_ast(node->children->next);
        }
        case NODE_DIV: {
            return interpret_ast(node->children) / interpret_ast(node->children->next);
        }
        case NODE_INT: {
            return *((int*)node->value);
        }
        default: {
            printf("unimplemented node: ");
            print_node(node);
            printf("\n");
            assert(false);
        }
    }
}

void print_node(ASTNode* node) {
    switch (node->type) {
        case NODE_INT: {
            printf("NodeInt(%d)", *((int*) node->value));
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
        case NODE_TERM: {
            printf("NodeTerm");
        } break;
        case NODE_MULT: {
            printf("NodeMult");
        } break;
        case NODE_DIV: {
            printf("NodeDiv");
        } break;
        default: {
            printf("UnknownNode");
        }
    }
    if (false && node->token) {
        printf(", Token: ");
        print_token(node->token);
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
