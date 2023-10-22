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
- Implicit multiplication ("5(4) = 20")
- Variables
- Multiple expressions ("a = sqrt(81); 2 * a" => 18)
- REPL
- Easy tests creation.
### Not implemented:

### Usage:
```
./main <input> [options] : run input
./main --repl : run in REPL mode
Options:
    --graph  Generate AST graph
    --debug  Prints debug information
```

### Build
`make` should the trick.

I guess this is buildable on any Linux system (idk much about compatibility and portability)

### Test files
A test file is a `.test` file located in the `tests` folder. Testcases can be defined in these files with a straightforward syntax.

#### Testcase
A test case can be defined like this:
`<expression> ~ <expected>`

A test case is single line.
You can add a comment with `#`. It must be the first character of the line.
