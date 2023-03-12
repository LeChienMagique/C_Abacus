#include "./ast_operations.h"
#include <stdlib.h>
#include <stdio.h>
#include <err.h>
#include <math.h>
#include <string.h>
#include <assert.h>

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

Result ast_do_binop(Result a, Result b, int (*funi) (int, int), double (*funf) (double, double)) {
    Result result = create_result(a, b);
    if (a.type == RESULT_FLOAT) {
        if (b.type == RESULT_FLOAT) {
            result.valf = funf(a.valf, b.valf);
        } else {
            result.valf = funf(a.valf, ((double) b.vali));
        }
    } else if (b.type == RESULT_FLOAT) {
        result.valf = funf(((double) a.vali), b.valf);
    } else {
        result.vali = funi(((int) a.vali), b.vali);
    }
    check_zero_result(&result);
    return result;
}

int addi(int a, int b) { return a + b; }
double addf(double a, double b) { return a + b; }

Result ast_add(Result a, Result b) {
    return ast_do_binop(a, b, addi, addf);
}

int subi(int a, int b) { return a - b; }
double subf(double a, double b) { return a - b; }

Result ast_sub(Result a, Result b) {
    return ast_do_binop(a, b, subi, subf);
}


int muli(int a, int b) { return a * b; }
double mulf(double a, double b) { return a * b; }

Result ast_mul(Result a, Result b) {
    return ast_do_binop(a, b, muli, mulf);
}


int divi(int a, int b) { return a / b; }
double divf(double a, double b) { return a / b; }

Result ast_div(Result a, Result b) {
    if (b.type == RESULT_FLOAT) {
        if (b.valf == 0) {
            errx(EXIT_FAILURE, "Division by zero");
        }
    } else if (b.vali == 0) {
        errx(EXIT_FAILURE, "Division by zero");
    }
    return ast_do_binop(a, b, divi, divf);
}

Result ast_exp(Result a, Result b) {
    Result result = create_result(a, b);
    if (a.type == RESULT_FLOAT) {
        if (b.type == RESULT_FLOAT) {
            if (a.valf == 0 && b.valf < 0) {
                errx(EXIT_FAILURE, "[ERROR] Cannot take 0 to a negative power");
            }
            result.valf = pow(a.valf, b.valf);
        } else {
            if (a.valf == 0 && b.vali < 0) {
                errx(EXIT_FAILURE, "[ERROR] Cannot take 0 to a negative power");
            }
            result.valf = pow(a.valf, b.vali);
        }
    } else if (b.type == RESULT_FLOAT) {
        if (a.vali == 0 && b.valf < 0) {
            errx(EXIT_FAILURE, "[ERROR] Cannot take 0 to a negative power");
        }

        result.valf = pow((double) a.vali, b.valf);
    } else {
        if (a.vali == 0 && b.vali < 0) {
            errx(EXIT_FAILURE, "[ERROR] Cannot take 0 to a negative power");
        }

        result.vali = (int) pow(a.vali, b.vali);
    }
    check_zero_result(&result);
    return result;
}

int imod(int a, int b) {
    return (int) fmod(a, b);
}

Result ast_mod(Result a, Result b) {
    return ast_do_binop(a, b, imod, fmod);
}

Result ast_equal(Result a, Result b) {
    Result result = {0};
    result.type = RESULT_INT;
    if (a.type == RESULT_FLOAT) {
        if (b.type == RESULT_FLOAT) {
            result.vali = a.valf == b.valf;
        } else {
            result.vali = a.valf == ((double) b.vali);
        }
    } else if (b.type == RESULT_FLOAT) {
        result.vali = ((double) a.vali) == b.valf;
    } else {
        result.vali = ((int) a.vali) == b.vali;
    }
    return result;
}

Result ast_neg(Result x) {
    return (Result) {
        .type = x.type,
        .vali = -(x.vali),
        .valf = -(x.valf)
    };
}

Result ast_evaluate_function(const char* func_name, int argc, Result* argv) {
    Result result = {0};
    if (strcmp(func_name, "sqrt") == 0) {
        double x;
        if (argv[0].type == RESULT_INT) {
            x = (double) argv[0].vali;
        } else {
            x = argv[0].valf;
        }

        if (x < 0) {
            errx(EXIT_FAILURE, "[ERROR] Domain error, sqrt(x) where x < 0");
        }

        result.type = RESULT_FLOAT;
        result.valf = sqrt(x);
    }
    else {
        assert(false && "unreachable");
    }
    assert(argc >= 0); // remove annoying unused parameter warning
    check_zero_result(&result);
    return result;
}
