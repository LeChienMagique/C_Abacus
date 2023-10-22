#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <assert.h>
#include <stdbool.h>

#include "./src/token.h"
#include "./src/ast.h"
#include "./src/runtime.h"

int main(int argc, char **argv)
{
    if (argc != 2)
    {
        return 1;
    }

    run(argv[1]);
}
