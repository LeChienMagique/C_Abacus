#include "./ast_operations.h"
#include <stdlib.h>
#include <stdio.h>

ASTNode* create_node(Token* token, int type);

ASTNode* create_result_node(ASTNode* a, ASTNode* b) {
    ASTNode* result;
    if (a->type == NODE_FLOAT || b->type == NODE_FLOAT) {
        // at least one float
        result = create_node(NULL, NODE_FLOAT);
    } else {
        // both int
        result = create_node(NULL, NODE_INT);
    }
    return result;
}

void node_values_to_double(ASTNode* a, ASTNode* b, double* a_val, double* b_val) {
    if (a->type == NODE_FLOAT) {
        *a_val = *((double*) a->value);
    } else {
        *a_val = (double) *((int*) a->value);
    }
    if (b->type == NODE_FLOAT) {
        *b_val = *((double*) b->value);
    } else {
        *b_val = (double) *((int*) b->value);
    }
}

void cast_node_result(ASTNode* result_node, double* result) {
    // result is freed if it is of incorrect type
    if (result_node->type == NODE_INT) {
        int* res_int = malloc(sizeof(int));
        *res_int = (int) *result;
        free(result);
        result_node->value = (void*) res_int;
    } else {
        result_node->value = (void*) result;
    }
}

ASTNode* ast_add(ASTNode* a, ASTNode* b) {
    ASTNode* result;
    if (a->type == NODE_FLOAT || b->type == NODE_FLOAT) {
        // at least one float
        result = create_node(NULL, NODE_FLOAT);
        double a_val, b_val;
        node_values_to_double(a, b, &a_val, &b_val);
        double* add_res = malloc(sizeof(double));
        *add_res = a_val + b_val;
        result->value = (void*) add_res;
    } else {
        // both int
        result = create_node(NULL, NODE_INT);
        int* add_res = malloc(sizeof(int));
        *add_res = *((int*) a->value) + *((int*) b->value);
        result->value = (void*) add_res;
    }
    return result;
}

ASTNode* ast_sub(ASTNode* a, ASTNode* b) {
    ASTNode* result;
    if (a->type == NODE_FLOAT || b->type == NODE_FLOAT) {
        // at least one float
        result = create_node(NULL, NODE_FLOAT);
        double a_val, b_val;
        node_values_to_double(a, b, &a_val, &b_val);
        double* add_res = malloc(sizeof(double));
        *add_res = a_val - b_val;
        result->value = (void*) add_res;
    } else {
        // both int
        result = create_node(NULL, NODE_INT);
        int* add_res = malloc(sizeof(int));
        *add_res = *((int*) a->value) - *((int*) b->value);
        result->value = (void*) add_res;
    }
    return result;
}

ASTNode* ast_mul(ASTNode* a, ASTNode* b) {
    ASTNode* result;
    if (a->type == NODE_FLOAT || b->type == NODE_FLOAT) {
        // at least one float
        result = create_node(NULL, NODE_FLOAT);
        double a_val, b_val;
        node_values_to_double(a, b, &a_val, &b_val);
        double* add_res = malloc(sizeof(double));
        *add_res = a_val * b_val;
        result->value = (void*) add_res;
    } else {
        // both int
        result = create_node(NULL, NODE_INT);
        int* add_res = malloc(sizeof(int));
        *add_res = *((int*) a->value) * (*((int*) b->value));
        result->value = (void*) add_res;
    }
    return result;
}

ASTNode* ast_div(ASTNode* a, ASTNode* b) {
    ASTNode* result;
    if (a->type == NODE_FLOAT || b->type == NODE_FLOAT) {
        // at least one float
        result = create_node(NULL, NODE_FLOAT);
        double a_val, b_val;
        node_values_to_double(a, b, &a_val, &b_val);
        double* add_res = malloc(sizeof(double));
        *add_res = a_val / b_val;
        result->value = (void*) add_res;
    } else {
        // both int
        result = create_node(NULL, NODE_INT);
        int* add_res = malloc(sizeof(int));
        *add_res = *((int*) a->value) / *((int*) b->value);
        result->value = (void*) add_res;
    }
    return result;
}

void ast_neg(ASTNode* node) {
    if (node->type == NODE_INT) {
        int value = *(int*) node->value;
        value *= -1;
        node->value = (void*) &value;
    } else if (node->type == NODE_FLOAT) {
        double value = *(double*) node->value;
        value *= -1;
        node->value = (void*) &value;
    }
}
