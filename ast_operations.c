#include "./ast_operations.h"
#include <stdlib.h>
#include <stdio.h>
#include <err.h>
#include <math.h>

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

void check_zero_result(Result* result) {
    if (result->type == RESULT_FLOAT && result->valf == 0) {
        result->type = RESULT_INT;
        result->vali = 0;
    }
}

Result ast_add(Result a, Result b) {
    Result result = create_result(a, b);
    if (a.type == RESULT_FLOAT) {
        if (b.type == RESULT_FLOAT) {
            result.valf = a.valf + b.valf;
        } else {
            result.valf = a.valf + ((double) b.vali);
        }
        result.type = RESULT_FLOAT;
    } else if (b.type == RESULT_FLOAT) {
        result.valf = ((double) a.vali) + b.valf;
    } else {
        result.vali = a.vali + b.vali;
    }
    check_zero_result(&result);
    return result;
}

Result ast_sub(Result a, Result b) {
    Result result = create_result(a, b);
    if (a.type == RESULT_FLOAT) {
        if (b.type == RESULT_FLOAT) {
            result.valf = a.valf - b.valf;
        } else {
            result.valf = a.valf - ((double) b.vali);
        }
    } else if (b.type == RESULT_FLOAT) {
        result.valf = ((double) a.vali) - b.valf;
    } else {
        result.vali = a.vali - b.vali;
    }
    check_zero_result(&result);
    return result;
}

Result ast_mul(Result a, Result b) {
    Result result = create_result(a, b);
    if (a.type == RESULT_FLOAT) {
        if (b.type == RESULT_FLOAT) {
            result.valf = a.valf * b.valf;
        } else {
            result.valf = a.valf * ((double) b.vali);
        }
    } else if (b.type == RESULT_FLOAT) {
        result.valf = ((double) a.vali) * b.valf;
    } else {
        result.vali = a.vali * b.vali;
    }
    check_zero_result(&result);
    return result;
}

Result ast_div(Result a, Result b) {
    Result result = create_result(a, b);
    if (a.type == RESULT_FLOAT) {
        if (b.type == RESULT_FLOAT) {
            result.valf = a.valf / b.valf;
        } else {
            result.valf = a.valf / ((double) b.vali);
        }
    } else if (b.type == RESULT_FLOAT) {
        result.valf = ((double) a.vali) / b.valf;
    } else {
        result.vali = a.vali / b.vali;
    }
    check_zero_result(&result);
    return result;
}

Result ast_exp(Result a, Result b) {
    Result result = create_result(a, b);
    if (a.type == RESULT_FLOAT) {
        if (b.type == RESULT_FLOAT) {
            result.valf = pow(a.valf, b.valf);
        } else {
            result.valf = pow(a.valf, (double) b.vali);
        }
    } else if (b.type == RESULT_FLOAT) {
        result.valf = pow((double) a.vali, b.valf);
    } else {
        result.vali = (int) pow(a.vali, b.vali);
    }
    check_zero_result(&result);
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
