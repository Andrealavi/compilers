# Small Compiler for a Minimal Functional Language

This version is used for adding new features to the compiler, in order to prepare for the **Compilers** exam.
As new features are added backward compatibility can be affected.

## Main Repository

For the stable version of this compiler, please visit the main repository:
[Minimal Functional Language Compiler - Main Repository](https://github.com/Andrealavi/compilers/tree/main/LFMCompilerLLVM)

## Changes

- Support to inline and multiline comments: (Commit: [ced766d](https://github.com/Andrealavi/compilers/commit/ced766de63c796556731813ddd3414c3a2734e4f))
	Added the following regexs in the `scanner.ll` file alongside with the code to manage them (in this case it is simple since they have to be ignored by the scanner)

  ```flex
  inline_comment \/\/.*
  multiline_comment \/\*.*\*\/

  ...

  {inline_comment} {}
  {multiline_comment} {}
  ```
- Support to global variables: (Commit: [b57aa71](https://github.com/Andrealavi/compilers/commit/b57aa71bff241f84ac15289042d20229e60e356a))
  - Added the `GlobalDefAST` class in order to implement global variables
  - Used the `GlobalVariable` LLVM class in order to create the global variable in the IR code
  - Added an if within `IdeExprAst` `codegen` method in order to check for global variable existence
  - Added a new example to test global variables
- Support to exponentiation: (Commit: [b5d2996](https://github.com/Andrealavi/compilers/commit/b5d2996290d490246c8166915db11de9a56f6c24))
  - Is now possible to elevate a number to a certain power using the following syntax: `expr ^ expr`
  - Modified `scanner.ll` and `parser.yy` in order to add the new token and production
  - Added a new node for the AST (`ExponentiationExprAST`)
  - Probably is not a perfectly efficient implementation but it does work
  - Probably it would have been better to have a proper function defined in the language, but this was much more formative
  - Added a new example to test it
- Support to for statement: (Commit: [40cd7d2](https://github.com/Andrealavi/compilers/commit/40cd7d2397937d951c7dcf8b4b894457c43a7127))
  - Added `ForExprAST` class in order to implement the for statement
  - Its syntax is similar to C++, so it is `for (int i = 0; i < 10; i + 1) expression end`
  - At the moment its usage is limited by the fact that do not exist assignments
  - It returns the value of the expression at the last iteration
  - Added a new example (`./code_examples/example_5.lfm`) to test it
  - The structure of the LLVM IR code is something like the following (example take from `./code_examples/example_5.lfm`) :
    ```LLVM
    entry:
      %bodyResult = alloca i32, align 4
      %i = alloca i32, align 4
      %x1 = alloca i32, align 4
      store i32 %x, ptr %x1, align 4
      store i32 1, ptr %i, align 4
      store i32 0, ptr %bodyResult, align 4
      br label %condition

    condition:                                        ; preds = %update, %entry
      %i2 = load i32, ptr %i, align 4
      %cmplt = icmp slt i32 %i2, 10
      br i1 %cmplt, label %loop, label %exit

    loop:                                             ; preds = %condition
      %x3 = load i32, ptr %x1, align 4
      %i4 = load i32, ptr %i, align 4
      %prod = mul nsw i32 %x3, %i4
      store i32 %prod, ptr %bodyResult, align 4
      br label %update

    update:                                           ; preds = %loop
      %i5 = load i32, ptr %i, align 4
      %sum = add nsw i32 %i5, 1
      store i32 %sum, ptr %i, align 4
      br label %condition

    exit:                                             ; preds = %condition
      %bodyResult6 = load i32, ptr %bodyResult, align 4
      ret i32 %bodyResult6
    ```
- Support to return instruction: (Commit: [26bdfc2](https://github.com/Andrealavi/compilers/commit/26bdfc247dc3ff797d7f53738d5c9ebd07ee7090))
  - Modified the grammar in order to make it possible to use `return` instruction in function
  - Is now mandatory to use the instruction in order to return a value, otherwise 0 will be returned
  - Added a new AST node to manage return expression (`RetExprAST`)
  - Modified the function `codegen` method:
    - Now the body is a vector of expressions
  - Added a new example (`./code_examples/example_6.lfm`) to test it
  - Modified a previous example (`./code_examples/example_5.lfm`) to present another use case
- Support to assignments: (Commit: [2fc22d2](https://github.com/Andrealavi/compilers/commit/2fc22d28b598afada6db7e9f73de0105c751f256))
  - Now is possible to use assignments not only in let expressions but also inside functions, fors and ifs
  - The assignments create a new variable if the identifier was not used, otherwise it overwrites the previous value
  - Updated `scanner.yy` grammar rules in order to allow for assignments in reductions
  - Added a new AST node (`AssignmentExprAST`) for representing assignments
  - Added a new example (`./code_examples/example_7.lfm`) to test assignments
- Added pipeline operator for composite functions: (Commit: [6af90ba](https://github.com/Andrealavi/compilers/commit/6af90ba4af48462dbcf7f909e910e65b3efa86a8))
  - Is now possible to concatenate functions using the operator `|>` (e.g. `f() |> g()`)
  - By using the pipeline operator the output of a function is passed as an argument to the following function
  - Added a new example (`./code_examples/example_8.lfm`) to test it
  - Added a new AST node (`PipExprAST`) that has a vector of `CallExprAST*`as its principal attribute
- Changed if and let syntax: (Commit: [c9c1567](https://github.com/Andrealavi/compilers/commit/c9c15673f6f9ea100c9c9b2eb2944809308d6193))
  - Now if and let can have multiple expressions within their blocks
  - If expressions blocks need to be enclosed in `{}`
  - Added a new example (`./code_examples/example_9.lfm`) to test it
- Added break instruction and solved a bug in if codegen method: (Commit: [25431ca](https://github.com/Andrealavi/compilers/commit/25431ca898458d396bde2ddddbeb787ba239fe84))
  - It is now possible to the break instruction to stop for loop iterations
  - Added a new AST node (`BreakExprAST`)
  - Added a new example to test it (`./code_examples/example_10.lfm`)
- Added function forward declaration: (Commit: [7d2c565](https://github.com/Andrealavi/compilers/commit/7d2c565e83fb3fa0cf25c8f4b48db9e732152eae))
  - It is now possible to first declare functions and define them later
  - To do so it was added `forward` instruction. It has to be put before the function prototype
  - Added a new example (`./code_examples/example_11.lfm`) to test forward declarations
  - To allow for forward declarations, only small changes to `PrototypeAST` and `FunctionAST`classes were made
- Implemented ternary operator (Short Hand if-else): (Commit: [ae4c895](https://github.com/Andrealavi/compilers/commit/ae4c895f4386960f9d3b943575563ff452bf321f))
  - Added a new AST node (`TernaryExprAST`)
  - Changed `parser.yy` and `scanner.ll` in order to properly manage the new construct
  - The syntax copies the C++ ternary operator (`condition ? trueExpr : falseExpr`)
  - Added a new example (`./code_examples/example_12.lfm`) to test it
- Implemented constants: (Commit: [11d6fbd](https://github.com/Andrealavi/compilers/commit/11d6fbd9a3e65c5edc2e2ea9249e321e06b7f82a))
  - Modified `driver` class in order to keep track of the constants
  - Added a vector of sets (`constantsScopes`) to manage scoping
  - Modified `FunctionAST`, `LetExprAST` and `AssignmentExprAST` to properly check and managing constants
  - Added a new example to test constants and scoping (`./code_examples/example_13.lfm`)
- Implemented DoWhile loop: (Commit: [0c7b124](https://github.com/Andrealavi/compilers/commit/0c7b1244b7cba87da4dd6839c9cb2f1bdb2d2aff))
  - Changed AST nodes structure by adding a `LoopExprAST` node from which for and while nodes inherit
  - Changed `scanner.ll` and `parser.yy` accordingly
  - Added a new example to test do while loop (`./code_examples/example_14.lfm`)
- Implemented integers arrays: (Commit:  [ec9cf36](https://github.com/Andrealavi/compilers/commit/ec9cf36d22cafd429c33a856d2988d5e3c8a7f17))
  - Created a new AST node (`ArrayExprAST`) to manage Array creation
  - Changed `scanner.ll` and `parser.yy` accordingly
  - Added a new example to test arrays (`./code_examples/example_15.lfm`)
- Implemented array comprehension: (Commit:  [16ddca2](https://github.com/Andrealavi/compilers/commit/16ddca24776192be5ae57e3f33b205eacde7c9dd))
  - Now it is possible to create arrays with the following syntax: `array a = {x for x in range(10)}`
  - Created a new AST node (`ComprExprAST`) to manage array comprehension
  - Changed `scanner.ll` and `parser.yy` accordingly
  - Created a new example to test it (`./code_examples/example_16.lfm`)
- Implemented for range loop: (Commit:  [0c070fe](https://github.com/Andrealavi/compilers/commit/0c070fe2fd6e22ee846cd40bd8bf1a381cbe3c04))
  - It is now possible to iterate over array using the following syntax: `for (element : array) expressions end`
  - Added a new AST node (`ForRangeExprAST`) to manage the operation
  - Added a new example to test it (`./code_examples/example_17.lfm`)
- Implemented switch statement: (Commit:  [57020c6](https://github.com/Andrealavi/compilers/commit/57020c6f0bf1a4401890103a0deedd06dea855ca))
  - Added switch statement using the specific `IRBuilder` method
  - Changed `scanner.ll` and `parser.yy` accordingly
  - Added three new AST nodes for managing switch statements (`CaseExprAST`, `DefaultCaseExprAST`, `SwitchExprAST`)
  - Created a new example to test it (`./code_examples/example_18.lfm`)
- Implemented custom data types (structs): (Commit:  [31ed17b](https://github.com/Andrealavi/compilers/commit/31ed17ba719a568db4a008cba166e74433fde77f))
  - Added support for creating and using struct-like data types with immediate instance creation
  - It is **not** possible to reuse the created struct data type
  - The syntax allows defining fields and their values in a single statement: `struct { x = 2, y = 3 } z`
  - Added field access using array-like syntax: `z[x]` to get or set field values
  - Created a new AST node (`StructExprAST`) to manage struct creation and field initialization
  - Modified `IdeExprAST` and `AssignmentExprAST` to handle struct field access and modification
  - Used LLVM's `StructType` and `CreateGEP` instructions for managing struct layout and field access
  - Added a new example to test structs (`./code_examples/example_19.lfm`)

**Note:** Features listed above may not be compatible with newer implementations. Check out the specific commit to test individual features.
