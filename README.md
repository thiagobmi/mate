<p align="center">
    <img src="https://github.com/user-attachments/assets/e3084dbb-7599-4416-aa6a-5e38039fe016" alt="nuvBench Logo" />
    <h3 align="center">Math Analysis Tool and Evaluator (MATE)</h3>
    <p align="center">
        <a href="#overview">Overview</a> &#183;
        <a href="#usage-as-a-terminal-application">CLI Usage</a> &#183;
        <a href="#usage-as-a-c-library">Library Usage</a> &#183;
        <a href="#how-it-works">How it works</a> &#183;
        <a href="#grammar">Grammar</a>
    </p>
</p>

---

## Overview
MATE is a versatile math expression evaluator that can be used in two ways:
1. As a **C library** for embedding in your applications
2. As a standalone **terminal application** for interactive math evaluation

It supports variables, functions, and complex mathematical expressions with a syntax similar to most programming languages.

```c
eval("2+2");                                 // Returns: 4
eval("cone_volume(r,h) = (1/3)*PI*r^2*h");   // Defines a function
eval("cone_volume(10,5)");                   // Returns: 523.59877559833330
```

## Usage as a Terminal Application

MATE can be compiled by entering the directory and running:
```bash
make
```

### Running MATE in the Terminal

After compilation, you can use the binary in two ways:

#### ***1. Interactive Mode***
Simply run the binary without arguments to enter interactive mode:
```bash
./mate
```

This launches MATE's interactive shell, similar to GNU calc. In this mode, you can:
- Enter expressions continuously
- Navigate expression history using `UP` and `DOWN` keys
- View all defined functions by typing `functions`
- View all defined variables by typing `variables`

![MATE functions command example](https://github.com/user-attachments/assets/e85d3580-1202-40e7-a663-e8e53d3274a0)

- You also can see the `abstract syntax tree` of a function by typing `showtree <function>`

![image](https://github.com/user-attachments/assets/990e5003-a40e-4dec-84c9-260880f1af25)


#### ***2. Single Expression Mode***
Pass expressions directly as command-line arguments:
```bash
./mate "expression"
```

### Command Line Options
- `-c / --config`: Path to the configuration file (default: `.mateconfig`)
- `-s / --simple-output`: Prints only the result without the input expressions

### Multiple Expressions
You can pass multiple expressions by separating them with semicolons:

```bash
./mate "circ_area(r)=PI*r^2; circ_area(10)"
```

## Usage as a C Library

When using MATE as a library in your C application, you'll use the `eval()` function to evaluate expressions:

### Variables
Define and use variables in your expressions:

```c
eval("PI = 3.14159265359");  // Define PI
eval("PI");                  // Returns: 3.14159265359
```

### Functions

#### Runtime-Defined Functions
Create functions during program execution:

```c
// Define a function that returns the maximum of two values
eval("max2(x,y) = (x>y) * x + (x<=y) * y");  // Returns: 0
eval("max2(10,30)");                         // Returns: 30
```

Inside a function, you can access:
- Local variables (defined in parentheses after the function name)
- Global variables (defined elsewhere in your program)

#### External C Functions
You can extend MATE with your own C functions:

```c
// Declaring factorial(x)
double fac(double x)
{
    if (x < 0)
        return NAN;
    if (x == 0 || x == 1)
        return 1;
    double result = 1;
    for (int i = 2; i <= (int)x; i++)
    {
        result *= i;
    }
    return result;
}

// Adding factorial to the program
// add_function(name, function_pointer, number_of_arguments);
add_function("fac", fac, 1);

// Then you can call fac in your program:
eval("fac(10)");  // Returns: 3628800
```

> **Note:** The return type and all arguments must have type `double`.

## How it works
MATE employs a recursive descent parser to compile input expressions into abstract syntax trees (ASTs). For example, the expression `sin x + 1/4` parses as:

![image](https://github.com/user-attachments/assets/1262d940-2019-4617-9d2c-98dee0f8ec31)

For functions, the following steps occur:
- **AST Construction:** The abstract syntax tree (AST) is built from the input expression.
- **Variable Storage:** Variables and their relationships are stored in memory.
- **Function Execution:** When the function is called, the AST is evaluated using the provided arguments as parameters.

## Grammar

```
<program>   =    <statement> {";" <statement>}
<statement> =    <assignment> | <expr>
<assignment> =   <variable> "=" <expr>
              |  <function-def>

<function-def> = <identifier> "(" [<parameter-list>] ")" "=" <expr>
<parameter-list> = <identifier> {"," <identifier>}
<expr>      =    <logical_or> {"=" <logical_or>}
<logical_or> =   <logical_and> {"||" <logical_and>}
<logical_and> =  <bitwise_and> {"&&" <bitwise_and>}
<bitwise_and> =  <equality> {"&" <equality>}
<equality>  =    <relational> {("==" | "!=") <relational>}
<relational> =   <additive> {("<" | ">" | "<=" | ">=") <additive>}
<additive>  =    <term> {("+" | "-") <term>}
<term>      =    <factor> {("*" | "/" | "%") <factor>}
<factor>    =    <power> {"^" <power>}
<power>     =    {("-" | "+")} <base>
<base>      =    <number>
              |  <variable>
              |  <function-call>
              |  "(" <expr> ")"

<function-call> = <identifier> "(" [<expr-list>] ")"
<expr-list> =    <expr> {"," <expr>}
<variable>  =    <identifier>
<identifier> =   <letter> {<letter> | <digit> | "_"}
<number>    =    <digit> {<digit>} ["." {<digit>}] [("e" | "E") ["+" | "-"] <digit> {<digit>}]
<letter>    =    "a" | ... | "z" | "A" | ... | "Z"
<digit>     =    "0" | ... | "9"
```
