# Small Interpreter for a Minimal Functional Language

This project is a simple interpreter for a minimal functional programming language, implemented using **Flex**, **Bison**, and **LLVM**. The interpreter parses, evaluates, and generates LLVM IR code for source code written in this custom language, showcasing the use of compiler construction tools and principles.

This code was uploaded by the professor for the **Compilers** course at University of Modena and Reggio Emilia.

## Features
- **Custom Functional Language**: A simple syntax designed for educational purposes
- **Abstract Syntax Tree (AST)**: Constructs an AST for semantic analysis
- **LLVM IR Generation**: Generates LLVM Intermediate Representation code
- **Type System**: Supports integers and booleans as primitive types

## Tools and Requirements
To build and use this project, the following tools are required:

- **Flex**: Used for lexical analysis
- **Bison**: Used for parsing and AST construction
- **Clang++**: A C++ compiler to build the project (with C++17 support)
- **LLVM 18**: Required for IR generation. If you have a different version installed, you MUST modify the Makefile to match your version number in the paths
- **Make**: To handle the build process using the provided `Makefile`

### File Structure
- **Makefile**: Automates the build process with LLVM dependencies
- **astscanner.ll**: Flex lexer for tokenizing the input
- **astparser.yy**: Bison parser for syntax analysis and AST creation
- **driver.hpp/cpp**: Driver classes for managing parsing and code generation
- **lfmc.cpp**: Main compiler entry point

## Language Features

### Data Types
- **Integer**: Basic numeric type
- **Boolean**: True/false values (represented as 1/0)

### Variables and Assignments
The language uses functional-style bindings through the `let` construct:

```lfm
let x = 10,
    y = 20 in
  x + y
end
```

### Expressions
- Arithmetic operations: `+`, `-`, `*`, `/`, `%`
- Comparison operations: `<`, `<=`, `>`, `>=`, `==`, `<>`
- Logical operations: `and`, `or`, `not`

### Functions
Supports recursive function definitions:
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
Multi-way conditional expressions:
```lfm
if  condition1 : expression1;
    condition2 : expression2;
    true : defaultExpression
end
```

### Input/Output
Predefined external functions:
- `external readint()`: Reads an integer from stdin
- `external printint(n)`: Prints an integer to stdout

## Abstract Syntax Tree (AST) Classes

### Base Classes
- **`RootAST`**: Base class for all AST nodes
- **`DefAST`**: Base class for definitions (functions, prototypes)
- **`ExprAST`**: Base class for expressions

### Expression Nodes
- **`NumberExprAST`**: Integer constants
- **`BoolConstAST`**: Boolean constants (0/1)
- **`IdeExprAST`**: Identifier references
- **`BinaryExprAST`**: Binary operations
- **`UnaryExprAST`**: Unary operations
- **`CallExprAST`**: Function calls
- **`IfExprAST`**: Conditional expressions
- **`LetExprAST`**: Local bindings

### Function-Related Nodes
- **`PrototypeAST`**: Function prototypes and external declarations
- **`FunctionAST`**: Complete function definitions

## LLVM Integration

### Core LLVM Classes Used
- **`LLVMContext`**: Manages global data and state
- **`Module`**: Contains all IR code for a compilation unit
- **`IRBuilder`**: Helper for generating LLVM IR instructions
- **`Type`**: Represents data types in LLVM
- **`Value`**: Base class for all values computed by IR instructions
- **`Function`**: Represents functions in LLVM IR
- **`BasicBlock`**: Contains sequences of IR instructions
- **`AllocaInst`**: Stack allocation instructions
- **`PHINode`**: Handles control flow merging

### Code Generation Process
1. **Module Setup**: Creates global LLVM context and module
2. **Function Generation**:
   - Creates function prototypes with appropriate types
   - Generates entry basic blocks
   - Allocates stack space for parameters
3. **Expression Generation**:
   - Recursively generates IR for expressions
   - Creates appropriate IR instructions for operations
   - Manages control flow with basic blocks
4. **Memory Management**:
   - Uses stack allocations for local variables
   - Manages variable scope in let expressions
5. **Control Flow**:
   - Generates conditional branches for if expressions
   - Uses PHI nodes to merge control flow paths

## Building and Running

### Compilation
```bash
make
```
Note: Make sure to modify the LLVM version in the Makefile to match your installed version.

### Running the Compiler
```bash
./lfmc [options] <source_file>
```

For example, you can try the compiler with the provided example programs in the `code_examples` folder:
```bash
# Run a simple example
./lfmc code_examples/example_1.lfm

# Generate LLVM IR for Euclid's algorithm
./lfmc -c code_examples/euclid.lfm

# Generate AST visualization in LaTeX
./lfmc -l code_examples/euclid.lfm
```

The `code_examples` folder contains various sample programs demonstrating different language features and programming patterns supported by the interpreter.

### Options
- `-p`: Enable parser debugging
- `-s`: Enable scanner debugging
- `-v`: Enable verbose output
- `-l`: Generate LaTeX visualization of AST
- `-c`: Generate LLVM IR code
- `-`: Read from stdin instead of file

### Cleaning Build Files
```bash
make clean
```
