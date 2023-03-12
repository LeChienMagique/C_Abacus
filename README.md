# CURRENTLY IN DEVELOPMENT

## Abacus in C
This project is a porting in C of my other Abacus project written in C#.
I originally wanted to try to make an AST using a recursive-descent parser in C and I thought it would be a good idea to also port Abacus to C.
This is my first time doing these kind of things with C so the code structure may be very sub-optimal but the goal is to improve by practice.

### Implemented:
- +, -, *, /, ^, %, ==, with operator precedence
- Unary + and -
- Parenthesis
- Floating point numbers
- Functions (sqrt, facto, fibo, max, min, isprime, gcd)

### Not implemented:
- Variables
- REPL

### Usage:
```
./main <input> [options] : run input
./main test run          : run tests
./main test save         : save expected results
Options:
    --graph  Generate AST graph
    --debug  Prints debug information
```
