# Small Interpreter for a Minimal Functional Language

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
