#ifndef RUNTIME_H
#define RUNTIME_H

#include "token.h"
#include "ast.h"

extern int GENERATE_GRAPH;
extern int DEBUG_MODE;

Result evaluate_input(const char* input);
void run(const char* input);

#endif /* ! RUNTIME_H */
