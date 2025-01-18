// Specifies the skeleton file 'lalr1.cc' to be used for generating the parser. This is a template for an LALR(1) parser written in C++.
// The '/* -*- C++ -*- */' tells editors like Emacs that this is a C++ file.
%skeleton "lalr1.cc" /* -*- C++ -*- */
%require "3.2"

%header
/*
   %define api.namespace {calc}   -> Changes the namespace (usually yy) to calc.
   %define api.location.file none -> Prevents the generation of location.hh file since locations are not
                                      used outside the parser.
   %define api.token.constructor  -> Defines functions in parser.hpp for returning tokens (make_XXX) from the scanner.
   %define api.value.type variant -> Defines the semantic type of different tokens (e.g., numbers, identifiers) using
                                      std::variant instead of a union.
*/
%define api.namespace {calc}
%define api.location.file none
%define api.token.constructor
%define api.value.type variant
%define parse.assert

/*
   %code requires -> Inserts code into parser.hpp.
   This is used to include external dependencies and declare the lexer prototype.
*/
%code requires {
  #include <string>
  #include <map>
  #include <cmath>

  // Define the prototype for the Flex lexer function.
  #define YY_DECL \
    calc::parser::symbol_type yylex ()
}

%locations // Enables location tracking in the parser.

%define parse.trace // Enables tracing for debugging.
%define parse.error verbose // Provides verbose error messages.

/*
   %code -> Inserts code into parser.cpp.
   This is used to declare and define variables and functions shared between the parser and scanner.
*/
%code {
  extern std::map<std::string, int> variables; // Stores variable names and their values.
  extern calc::location location; // Tracks the current location in the input.
  extern int result; // Stores the final result of an expression.
  YY_DECL; // Declare the lexer function.
}

%define api.token.prefix {TOK_} // Prefix for token names.
%token
  END  0  "end of file"
  ASSIGN  "="
  MINUS   "-"
  PLUS    "+"
  STAR    "*"
  SLASH   "/"
  LPAREN  "("
  RPAREN  ")"
  POWER   "^"
  SQRT    "sqrt"
;

%token <std::string> IDENTIFIER "identifier" // Token for variable names or keywords.
%token <int> NUMBER "number" // Token for integer values.
%nterm  <int> exp // Non-terminal for expressions.

/*
   %printer -> Custom printer for debugging or tracing. The provided code block is invoked whenever the
   parser needs to print a symbol. <*> applies to symbols with any semantic value.
*/
%printer { yyoutput << $$; } <*>;

%%
%start unit; // Define the start symbol for the grammar.

// Grammar rules.
unit: assignments exp  { result = $2; }; // Final result is stored in 'result'.

assignments:
  %empty                 {} // Empty assignments block.
| assignments assignment {}; // Recursively parse multiple assignments.

assignment:
  "identifier" "=" exp { variables[$1] = $3; }; // Assign a value to a variable.

%left "+" "-"; // Define left-associative operators for addition and subtraction.
%left "*" "/"; // Define left-associative operators for multiplication and division.
%right "^";      // Define right-associative operator for exponentiation.

exp:
  exp "+" exp   { $$ = $1 + $3; } // Addition.
| exp "-" exp   { $$ = $1 - $3; } // Subtraction.
| exp "*" exp   { $$ = $1 * $3; } // Multiplication.
| exp "/" exp   { $$ = $1 / $3; } // Division.
| exp "^" exp   { $$ = pow($1, $3); } // Exponentiation.
| "(" exp ")"   { $$ = $2; } // Parentheses for grouping.
| "identifier"  { $$ = variables[$1]; } // Retrieve variable value.
| "number"      { $$ = $1; } // Use number value directly.
| "sqrt" "(" exp ")" { $$ = sqrt($3); } // Square root operation.
%%

// Error handling function for syntax errors.
void
calc::parser::error (const location_type& l, const std::string& m)
{
  std::cerr << l << ": " << m << '\n';
}
