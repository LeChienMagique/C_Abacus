#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <err.h>
#include <assert.h>
#include "./ast.h"

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
    // printf("(");
    print_node(root);
    // printf(")");
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

/*
1 + 1

program -> expr -> ((term -> operand -> number 1) (operator +) (term -> operand -> number 1))

program = expr

expr = [+ | -] term {operator term}

term = operand {operator operand}

operand = number
        | ( expr )

operator = + | - | * | /
*/

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
        printf("Expected parenthesis followed by expression but got: ");
        print_token(*tokens);
        printf("\n");
        assert(false);
    }

    op = ast_next_expr(tokens);
    if (op) {
        return op;
    }
    assert(false && "unreachable");
}

ASTNode* ast_next_operator(Token** tokens) {
    if (*tokens == NULL) {
        return NULL;
    }

    ASTNode* optor = create_node(*tokens, -1);
    switch ((*tokens)->type) {
        case TOKEN_PLUS: {
            optor->type = NODE_PLUS;
            optor->value = "+";
        } break;
        case TOKEN_MINUS: {
            optor->type = NODE_MINUS;
            optor->value = "-";
        } break;
        case TOKEN_MULT: {
            optor->type = NODE_MULT;
            optor->value = "*";
        } break;
        default: {
            free(optor);
            return NULL;
        }
    }
    *tokens = (*tokens)->next;
    return optor;
}

ASTNode* ast_next_term(Token** tokens) {
    if (*tokens == NULL) {
        return NULL;
    }
    // term = operand {operator operand}
    ASTNode* term = create_node(NULL, NODE_TERM);

    // operand
    ASTNode* operand = ast_next_operand(tokens);
    if (operand == NULL) {
        printf("Expected operand got: ");
        print_token(*tokens);
        printf("\n");
        assert(false);
    }
    append_child(term, operand);

    // {operator operand}
    ASTNode* optor = ast_next_operator(tokens);
    while (optor != NULL) {
        optor->children = term->children;
        term->children = NULL;
        append_child(term, optor);

        operand = ast_next_operand(tokens);
        append_child(optor, operand);

        optor = ast_next_operator(tokens);
    }
    return term;
}


ASTNode* ast_next_unary(Token** tokens) {
    ASTNode* node;
    switch ((*tokens)->type) {
        case TOKEN_PLUS: {
            node = create_node(*tokens, NODE_UPLUS);
        } break;
        case TOKEN_MINUS: {
            node = create_node(*tokens, NODE_UMINUS);
        } break;
        default: {
            return NULL;
        }
    }
    *tokens = (*tokens)->next;
    return node;
}

ASTNode* ast_next_expr(Token** tokens) {
    if (*tokens == NULL) {
        return NULL;
    }

    // expr = [+ | -] term {operator term}
    ASTNode* expr = create_node(NULL, NODE_EXPR);

    // [+ | -]
    ASTNode* unary = ast_next_unary(tokens);
    if (unary) {
        append_child(unary, expr);
    }

    // term
    ASTNode* term = ast_next_term(tokens);
    if (term == NULL) {
        printf("Expected term got: ");
        print_token(*tokens);
        assert(false);
    }
    append_child(expr, term);

    // {operator term}
    ASTNode* optor = ast_next_operator(tokens);
    while (optor) {
        optor->children = expr->children;
        expr->children = NULL;
        append_child(expr, optor);

        term = ast_next_term(tokens);
        append_child(optor, term);

        optor = ast_next_operator(tokens);
    }
    return unary ? unary : expr;
}

ASTNode* build_AST(Token** tokens) {
    ASTNode* ast = ast_next_expr(tokens);
    if (*tokens) {
        assert(false && "[ERROR] leftover tokens");
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
