# Small Compiler for a Minimal Functional Language

This version is used for adding new features to the compiler, in order to prepare for the **Compilers** exam.

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
- Support to exponentiation
  - Is now possible to elevate a number to a certain power using the following syntax: `expr ^ expr`
  - Modified `scanner.ll` and `parser.yy` in order to add the new token and production
  - Added a new node for the AST (`ExponentiationExprAST`)
  - Probably is not a perfectly efficient implementation but it does work
  - Probably it would have been better to have a proper function defined in the language, but this was much more formative
  - Added a new example to test it
