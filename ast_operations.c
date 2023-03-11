#include "./ast_operations.h"
#include <stdlib.h>
#include <stdio.h>
#include <err.h>

Result create_result(Result a, Result b) {
    Result result = {0};
    if (a.type == RESULT_FLOAT || b.type == RESULT_FLOAT) {
        // at least one float
        result.type = RESULT_FLOAT;
    } else {
        // both int
        result.type = RESULT_INT;
    }
    return result;
}

Result create_result_from_node(ASTNode* node) {
    Result result = {0};
    if (node->type == NODE_INT) {
        result.type = RESULT_INT;
        result.vali = *((int*) node->value);
    } else if (node->type == NODE_FLOAT) {
        result.type = RESULT_FLOAT;
        result.valf = *((double*) node->value);
    } else {
        errx(EXIT_FAILURE, "Unknown value type");
    }
    return result;
}

Result ast_add(Result a, Result b) {
    Result result = {0};
    if (a.type == RESULT_FLOAT || b.type == RESULT_FLOAT) {
        // at least one float
        result.valf = a.valf + b.valf;
    } else {
        // both int
        result.vali = a.vali + b.vali;
    }
    return result;
}

Result ast_sub(Result a, Result b) {
    Result result = create_result(a, b);
    if (a.type == RESULT_FLOAT || b.type == RESULT_FLOAT) {
        // at least one float
        result.valf = a.valf - b.valf;
    } else {
        // both int
        result.vali = a.vali - b.vali;
    }
    return result;
}

Result ast_mul(Result a, Result b) {
    Result result = create_result(a, b);
    if (a.type == RESULT_FLOAT || b.type == RESULT_FLOAT) {
        // at least one float
        result.valf = a.valf * b.valf;
    } else {
        // both int
        result.vali = a.vali * b.vali;
    }
    return result;
}

Result ast_div(Result a, Result b) {
    Result result = create_result(a, b);
    if (a.type == RESULT_FLOAT || b.type == RESULT_FLOAT) {
        // at least one float
        result.valf = a.valf / b.valf;
    } else {
        // both int
        result.vali = a.vali / b.vali;
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
    } else {
        errx(1, "unreachable");
    }
}
