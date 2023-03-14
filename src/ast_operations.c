#include "./ast_operations.h"
#include <stdlib.h>
#include <stdio.h>
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
        fprintf(stderr, "unreachable");
        exit(1);
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
            fprintf(stderr, "Division by zero");
            exit(1);
        }
    } else if (b.vali == 0) {
        fprintf(stderr, "Division by zero");
        exit(1);
    }
    return ast_do_binop(a, b, divi, divf);
}

Result ast_exp(Result a, Result b) {
    Result result = create_result(a, b);
    if (a.type == RESULT_FLOAT) {
        if (b.type == RESULT_FLOAT) {
            if (a.valf == 0 && b.valf < 0) {
                fprintf(stderr, "[ERROR] Cannot take 0 to a negative power");
                exit(1);
            }
            result.valf = pow(a.valf, b.valf);
        } else {
            if (a.valf == 0 && b.vali < 0) {
                fprintf(stderr, "[ERROR] Cannot take 0 to a negative power");
                exit(1);
            }
            result.valf = pow(a.valf, b.vali);
        }
    } else if (b.type == RESULT_FLOAT) {
        if (a.vali == 0 && b.valf < 0) {
            fprintf(stderr, "[ERROR] Cannot take 0 to a negative power");
            exit(1);
        }

        result.valf = pow((double) a.vali, b.valf);
    } else {
        if (a.vali == 0 && b.vali < 0) {
            fprintf(stderr, "[ERROR] Cannot take 0 to a negative power");
            exit(1);
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

Result ast_sqrt(Result x) {
    Result result = {0};
    result.type = RESULT_FLOAT;

    double x_val;
    if (x.type == RESULT_INT) {
        x_val = (double) x.vali;
    } else {
        x_val = x.valf;
    }

    if (x_val < 0) {
        fprintf(stderr, "[ERROR] Domain error, sqrt(x) where x < 0");
        exit(1);
    }

    result.valf = sqrt(x_val);
    return result;
}

int facti(int n) {
    if (n == 0) {
        return 1;
    }
    return n * facti(n - 1);
}

Result ast_facto(Result x) {
    Result result = {0};
    if (x.type == RESULT_INT) {
        if (x.vali < 0) {
            fprintf(stderr, "[ERROR] Domain error, facto(x) where x < 0");
            exit(1);
        }
        result.type = RESULT_INT;
        result.vali = facti(x.vali);

    } else {
        if (x.valf < -1) {
            fprintf(stderr, "[ERROR] Domain error, facto(x) where x < 0");
            exit(1);
        }
        result.type = RESULT_FLOAT;
        result.valf = tgamma(x.valf + 1);
    }
    check_zero_result(&result);
    return result;
}

int fibo(int n) {
    if (n == 0 || n == 1) {
        return n;
    }
    return fibo(n - 1) + fibo(n - 2);
}

Result ast_fibo(Result n) {
    Result result = {
        .type = RESULT_INT
    };

    int n_val;
    if (n.type == RESULT_FLOAT) {
        n_val = (int) n.valf;
    } else {
        n_val = n.vali;
    }

    if (n_val < 0) {
        fprintf(stderr, "[ERROR] Domain error fibo(n) where n < 0");
        exit(1);
    }

    result.vali = fibo(n_val);
    return result;
}

int imin(int a, int b) { return (int) fmin(a, b); }
Result ast_min(Result a, Result b) {
    return ast_do_binop(a, b, imin, fmin);
}


int imax(int a, int b) { return (int) fmax(a, b); }
Result ast_max(Result a, Result b) {
    return ast_do_binop(a, b, imax, fmax);
}


int is_prime(int n) {
    if (n <= 1) { return 0; }
    if (n == 2) { return 1; }
    if (n % 2 == 0) { return 0; }

    int limit = (int) floor(sqrt(n));

    for (int i = 3; i <= limit; i++) {
        if (n % i == 0) {
            return 0;
        }
    }
    return 1;
}
Result ast_isprime(Result n) {
    Result result = {
        .type = RESULT_INT
    };

    if (n.type == RESULT_FLOAT) {
        result.vali = 0;
    } else {
        result.vali = is_prime(n.vali);
    }

    return result;
}

int compute_gcd(int a, int b) {
    while (a != 0 && b != 0) {
        if (a > b) {
            a %= b;
        } else {
            b %= a;
        }
    }
    return a | b;
}

Result ast_gcd(Result a, Result b) {
    int a_val, b_val;
    if (a.type == RESULT_FLOAT) {
        a_val = (int) a.valf;
    } else {
        a_val = a.vali;
    }
    if (b.type == RESULT_FLOAT) {
        b_val = (int) b.valf;
    } else {
        b_val = b.vali;
    }

    int gcd;
    if (a_val < 0 && b_val < 0) {
        a_val *= -1;
        b_val *= -1;
        gcd = (-1) * compute_gcd(a_val, b_val);
    } else {
        gcd = compute_gcd(a_val, b_val);
    }

    return (Result) {
        .type = RESULT_INT,
        .vali = gcd
    };
}


Result ast_evaluate_function(const char* func_name, int argc, Result* argv) {
    if (strcmp(func_name, "sqrt") == 0) {
        return ast_sqrt(argv[0]);
    }
    else if (strcmp(func_name, "facto") == 0) {
        return ast_facto(argv[0]);
    }
    else if (strcmp(func_name, "fibo") == 0) {
        return ast_fibo(argv[0]);
    }
    else if (strcmp(func_name, "max") == 0) {
        return ast_max(argv[0], argv[1]);
    }
    else if (strcmp(func_name, "min") == 0) {
        return ast_min(argv[0], argv[1]);
    }
    else if (strcmp(func_name, "isprime") == 0) {
        return ast_isprime(argv[0]);
    }
    else if (strcmp(func_name, "gcd") == 0) {
        return ast_gcd(argv[0], argv[1]);
    }
    else {
        assert(false && "unreachable");
    }
    assert(argc >= 0); // remove annoying unused parameter warning
}
