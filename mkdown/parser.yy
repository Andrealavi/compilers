%skeleton "lalr1.cc" /* -*- C++ -*- */
%require "3.2"

%header

%define api.namespace {text}
%define api.location.file none
%define api.token.constructor
%define api.value.type variant
%define parse.assert

%code requires {
    #include <string>

    // Define the prototype for the Flex lexer function.
    #define YY_DECL text::parser::symbol_type yylex ()
}

%locations

%define parse.trace
%define parse.error verbose

%code {
  extern text::location location; // Tracks the current location in the input.
  extern std::string result; // Stores the final result of an expression.
  YY_DECL; // Declare the lexer function.
}

%token
  END  0    "end of file"
  STAR      "*"
  MINUS     "-"
  SEPARATOR ";"
  LSPAREN   "["
  RSPAREN   "]"
;

%token <std::string> WORD "word";
%token <std::string> LINK "link";
%nterm <std::string> phrases;
%nterm <std::string> phrase;
%nterm <std::string> words;

%printer { yyoutput << $$; } <*>;

%%

%start paragraph;

paragraph:
    phrases     { result = $1; };

phrases:
    %empty          {};
|   phrases phrase  { $$ = $1 + " " + $2; };

phrase:
    "*" words "*" ";"               { $$ = "\033[1m" + $2 + "\033[0m"; };
|   "-" words "-" ";"               { $$ = "\033[3m" + $2 + "\033[0m"; };
|   "[" "link" "]" "[" words "]"    { $$ = "\033]8;;" + $2 + "\033\\" + $5 + "\033]8;;\033\\"; }
|   words ";"                       { $$ = $1; };

words:
    %empty          {};
|   words "word"    { $$ = $1 + " " + $2; };

%%

void text::parser::error (const location_type& l, const std::string& m) {
    std::cerr << l << ": " << m << '\n';
}
