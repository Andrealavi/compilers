# Small Compiler for a Minimal Functional Language

This version is used for adding new features to the compiler, in order to prepare for the **Compilers** exam.
As new features are added backward compatibility can be affected.

## Main Repository

For the stable version of this compiler, please visit the main repository:
[Minimal Functional Language Compiler - Main Repository](https://github.com/Andrealavi/compilers/tree/main/LFMCompilerLLVM)

## Changes

- Support to inline and multiline comments:
	Added the following regexs in the `scanner.ll` file alongside with the code to manage them (in this case it is simple since they have to be ignored by the scanner)

  ```flex
  inline_comment \/\/[ a-zA-Z0-9'*.]*
  multiline_comment \/\*[\t\na-zA-Z0-9 .*]*\*\/

  ...

  {inline_comment} {}
  {multiline_comment} {}
  ```
- Support to global variables:
  - Added the `GlobalDefAST` class in order to implement global variables
  - Used the `GlobalVariable` LLVM class in order to create the global variable in the IR code
  - Added an if within `IdeExprAst` `codegen` method in order to check for global variable existence
  - Added a new example to test global variables
- Support to exponentiation:
  - Is now possible to elevate a number to a certain power using the following syntax: `expr ^ expr`
  - Modified `scanner.ll` and `parser.yy` in order to add the new token and production
  - Added a new node for the AST (`ExponentiationExprAST`)
  - Probably is not a perfectly efficient implementation but it does work
  - Probably it would have been better to have a proper function defined in the language, but this was much more formative
  - Added a new example to test it
- Support to for statement:
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
- Support to return instruction:
  - Modified the grammar in order to make it possible to use `return` instruction in function
  - Is now mandatory to use the instruction in order to return a value, otherwise 0 will be returned
  - Added a new AST node to manage return expression (`RetExprAST`)
  - Modified the function `codegen` method:
    - Now the body is a vector of expressions
  - Added a new example (`./code_examples/example_6.lfm`) to test it
  - Modified a previous example (`./code_examples/example_5.lfm`) to present another use case
- Support to assignments:
  - Now is possible to use assignments not only in let expressions but also inside functions, fors and ifs
  - The assignments create a new variable if the identifier was not used, otherwise it overwrites the previous value
  - Updated `scanner.yy` grammar rules in order to allow for assignments in reductions
  - Added a new AST node (`AssignmentExprAST`) for representing assignments
  - Added a new example (`./code_examples/example_7.lfm`) to test assignments
