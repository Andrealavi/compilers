# Small Interpreter for a Minimal Functional Language

This project is a simple interpreter for a minimal functional programming language, implemented using **Flex**, **Bison**, and **Clang**. The interpreter parses and evaluates source code written in this custom language, showcasing the use of compiler construction tools and principles.

This code was uploaded by the professor for the **Compilers** course at University of Modena and Reggio Emilia.

## Features
- **Custom Functional Language**: A simple syntax designed for educational purposes.
- **Abstract Syntax Tree (AST)**: Constructs an AST for semantic analysis.
- **Interpreter**: Evaluates expressions and handles basic functional constructs.

---

## Tools and Requirements
To build and use this project, the following tools are required:

- **Flex**: Used for lexical analysis.
- **Bison**: Used for parsing and AST construction.
- **Clang or GCC**: A C++ compiler to build the project.
- **Make**: To handle the build process using the provided `Makefile`.

### File Structure
- **Makefile**: Automates the build process.
- **astscanner.ll**: Flex lexer for tokenizing the input.
- **astparser.yy**: Bison parser for syntax analysis and AST creation.
- **astdriver.hpp**: Driver header for managing parsing and evaluation.

---

## Language Features
The custom functional language supported by this interpreter includes:

### Data Types
- **Integer**: The only supported data type.

### Variables and Assignments
- The language does not have variables in the traditional sense or assignment operations. However, it includes a `let` construct, a common feature in functional programming languages, which allows defining a block preceded by one or more **bindings** of values to identifiers:

  ```lfm
  let x = 10,
      y = 20 in
  x + y
  end
  ```

- The value of an identifier cannot be changed within the `let` block, and identifiers are not accessible outside the block.

### Expressions
- Supports arithmetic expressions with operators like `+`, `-`, `*`, and `/`.
  ```lfm
  let result = x + y * 2 in
  result
  end
  ```

### Functions
- The language supports recursive functions for control flow. Functions can be defined and called as follows:
  ```lfm
  function fibo(n)
    if n <= 1 : n;
      true :  let x = fibo(n - 2),
                  y = fibo(n - 1) in
                  x + y
              end
    end
  end
  ```

### Control Structures
- **Multi-way Conditional Constructs**:
  ```lfm
  if  condition1 : expression1;
      condition2 : expression2;
      true : defaultExpression
  end
  ```

### Input and Output
- External I/O functionality is provided via predefined functions:
  - `external readint()`: Reads an integer from standard input.
  - `external printint(n)`: Prints an integer to standard output.

### Example Program
To compute and print the Fibonacci number for a given input:
```lfm
external readint();
external printint(n);

function fibo(n)
  if n <= 1 : n;
    true :  let x = fibo(n - 2),
                y = fibo(n - 1) in
                x + y
            end
  end
end

let n = readint(),
    F = fibo(n) in
    printint(F)
end
```
---

## Abstract Syntax Tree (AST) Classes

The interpreter relies on an extensive class hierarchy to represent elements of the AST:

### Base Classes
- **`RootAST`**: The base class for all AST nodes, with virtual methods for traversing and evaluating elements.

### Definition and Expression Nodes
- **`DefAST`**: Base class for all definition nodes.
- **`ExprAST`**: Base class for all expression nodes.

### Specific Expression Nodes
- **`NumberExprAST`**: Represents numeric constants.
- **`IdeExprAST`**: Represents identifier references.
- **`BinaryExprAST`**: Represents binary operators.
- **`UnaryExprAST`**: Represents unary operators.
- **`CallExprAST`**: Represents function calls, storing the callee and arguments.
- **`IfExprAST`**: Represents the conditional construct.
- **`LetExprAST`**: Represents blocks with locally bound identifiers.

### Functionality Nodes
- **`PrototypeAST`**: Represents function prototypes, including parameter information.
- **`FunctionAST`**: Represents function definitions, comprising a prototype and a body.

### Driver Class
- **`driver`**: Organizes and manages the compilation process, handling scanning and parsing.

---

## Example Source Code
To test the interpreter, create a source file (e.g., `example.lfm`) and write the following code:

```lfm
let x = 10,
    y = 20 in
    printint(x + y)
end
```

### Steps to Execute
1. Save the code in a file with the `.lfm` extension (e.g., `example.lfm`).
2. Compile the interpreter using the instructions below.
3. Run the interpreter, passing the filename as an argument.

---

## How to Compile
The project includes a `Makefile` to streamline the build process:

1. Open a terminal and navigate to the project directory.
2. Run the following command to compile:

   ```bash
   make
   ```

   This will generate an executable named `interpreter` in the current directory.

3. (Optional) To clean up the build files, run:

   ```bash
   make clean
   ```

---

## Using the Interpreter
After building the project, you can use the interpreter as follows:

```bash
./interpreter [options] <source_file>
```

### Available Arguments
- `<source_file>`: Path to the source file containing the custom language code.
- `-h` or `--help`: Displays a help message.
- `-d` or `--debug`: Enables debug mode for verbose output during execution.
- `-` (dash): If passed as the source file argument, the interpreter will read input directly from the standard input (stdin).

---

## Additional Notes
- The project demonstrates the integration of Flex and Bison for lexical and syntactic analysis.
- The interpreter is designed to be extendable; you can add more language constructs by modifying `astparser.yy` and `astscanner.ll`.
