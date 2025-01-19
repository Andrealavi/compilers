%skeleton "lalr1.cc" /* -*- C++ -*- */
%require "3.2"
%defines /* Creates header file */

/*
    api.token.constructor:  Enables handling of complete symbols (type, value, location) in the scanner.
    api.location.file: Specifies the file for default location/position types in C++.
        `none` generates definitions in the header (if %header) or implementation file.
    api.value.type:  Sets the type for semantic values, `variant` allows different types of data to be stored.
    parse.assert: Enables runtime assertions to catch invalid parser usage.
        It is important when using C++ variants in order to assert that object are constructed and destroyed properly.
    parse.trace:  Enables run-time tracing of parser activity using the `yydebug` variable.
    parse.error:  Controls the verbosity of syntax error messages; `verbose` provides detailed error information.
*/
%define api.token.constructor
%define api.location.file none
%define api.value.type variant
%define parse.assert
%define parse.trace
%define parse.error verbose

%code requires {
    //#include <exception>

    /*
        Imports classes defined in the driver header file in order to properly act when applying reductions
    */

    class driver;
    class RootAST;
    class ExprAST;
    class FunctionAST;
    class PrototypeAST;
    class IfExprAST;
    class LetExprAST;
    class DefAST;

    // Tell Flex the lexer's prototype ...
    # define YY_DECL \
    yy::parser::symbol_type yylex (driver& drv)
}

/*
    Specifies additional arguments for the yylex/yyparse functions.
    In this case a reference to a driver object is added
*/
%param { driver& drv }

%locations // Generates the code for the location class

%code {
    # include "astdriver.hpp"
    YY_DECL;
}

%define api.token.prefix {TOK_}
%token
    EOF  0  "end of file"
    COLON      ":"
    SEMICOLON  ";"
    COMMA      ","
    MINUS      "-"
    PLUS       "+"
    STAR       "*"
    SLASH      "/"
    MOD        "%"
    LPAREN     "("
    RPAREN     ")"
    ALT        "|"
    LE         "<="
    LT         "<"
    EQ         "=="
    NEQ        "<>"
    GE         ">="
    GT         ">"
    BIND       "="
    TRUE       "true"
    FALSE      "false"
    EXTERN     "external"
    DEF        "function"
    IF         "if"
    AND        "and"
    OR         "or"
    NOT        "not"
    LET        "let"
    IN         "in"
    END        "end"
;

/*
	Defines tokens types
*/
%token <std::string> IDENTIFIER "id"
%token <int> NUMBER "number"

/* deflist is a list of functions */
%type <std::vector<DefAST*>> deflist

%type <DefAST*> def
%type <ExprAST*> expr
%type <ExprAST*> boolexpr
%type <FunctionAST*> funcdef
%type <PrototypeAST*> extdef
%type <PrototypeAST*> prototype
%type <std::vector<std::string>> params
%type <std::vector<ExprAST*>> arglist
%type <std::vector<ExprAST*>> args
%type <IfExprAST*> condexpr
%type <LetExprAST*> letexpr
%type <std::vector<std::pair<ExprAST*, ExprAST*>>> pairs;
%type <std::pair<ExprAST*, ExprAST*>> pair;
%type <std::vector<std::pair<std::string, ExprAST*>>> bindings;
%type <std::pair<std::string, ExprAST*>> binding;
%type <ExprAST*> literal
%type <ExprAST*> relexpr

%%

%start startsymb;

/* A LFM source file is made by a list of functions, with a main function that will be executed */
startsymb:
	deflist               { drv.root = $1; };

deflist:
	def deflist           { $2.insert($2.begin(), $1); $$ = $2; }
|	def                   { $$ = {$1}; };

def:
	extdef                { $$ = $1; }
|	funcdef               { $$ = $1; };

extdef:
	"external" prototype  { $$ = $2; };

funcdef:
	"function" prototype expr "end"  { $$ = new FunctionAST($2, $3); };

prototype:
	"id" "(" params ")"   { $$ = new PrototypeAST($1, $3); };

params:
	%empty                { std::vector<std::string> params; $$ = params; }
|	"id" params           { $2.insert($2.begin(), $1); $$ = $2; };

/* Defines rules for operators associativity and precedence*/
%nonassoc "<" "==" "<>" "<=" ">" ">=";
%left "+" "-";
%left "*" "/" "%";
%nonassoc UMINUS;
%left "or";
%left "and";
%nonassoc NEGATE;

expr:
	expr "+" expr          { $$ = new BinaryExprAST("+",$1, $3); }
|	expr "-" expr          { $$ = new BinaryExprAST("-",$1, $3); }
|	expr "*" expr          { $$ = new BinaryExprAST("*",$1, $3); }
|	expr "/" expr          { $$ = new BinaryExprAST("/",$1, $3); }
|	expr "%" expr          { $$ = new BinaryExprAST("%",$1, $3); }
|	"-" expr %prec UMINUS  { $$ = new UnaryExprAST("-",$2); }
|	"(" expr ")"           { $$ = $2; }
|	"id"                   { $$ = new IdeExprAST($1); }
|	"id" "(" arglist ")"   { $$ = new CallExprAST($1, $3); }
|	"number"               { $$ = new NumberExprAST($1); }
|	condexpr               { $$ = $1; }
|	letexpr                { $$ = $1; };

arglist:
	%empty                 { std::vector<ExprAST*> args; $$ = args; }
|	args                   { $$ = $1; };

args:
	expr                   { $$ = {$1}; }
|	expr "," args          { $3.insert($3.begin(), $1); $$ = $3; };

condexpr:
 "if" pairs "end"        { $$ = new IfExprAST($2); };

pairs:
	pair                   { $$ = {$1}; }
|	pair ";" pairs         { $3.insert($3.begin(), $1); $$ = $3; };

pair:
	boolexpr ":" expr      { std::pair<ExprAST*,ExprAST*> P ($1,$3); $$ = P; };

boolexpr:
	boolexpr "and" boolexpr { $$ = new BinaryExprAST("and", $1, $3); }
|	boolexpr "or" boolexpr  { $$ = new BinaryExprAST("or", $1, $3); }
|	"not" boolexpr  %prec NEGATE { $$ = new UnaryExprAST("not", $2); }
|	literal                 { $$ = $1; }
|	relexpr                 { $$ = $1; };

literal:
	"true"                  { $$ = new NumberExprAST(1); }
|	"false"                 { $$ = new NumberExprAST(0); };

relexpr:
	expr "<"  expr          { $$ = new BinaryExprAST("<", $1, $3);  }
|	expr "==" expr          { $$ = new BinaryExprAST("==", $1, $3); }
|	expr "<>" expr          { $$ = new BinaryExprAST("<>", $1, $3); }
|	expr "<=" expr          { $$ = new BinaryExprAST("<=", $1, $3); }
|	expr ">"  expr          { $$ = new BinaryExprAST(">", $1, $3); }
|	expr ">=" expr          { $$ = new BinaryExprAST(">=", $1, $3); }

letexpr:
	"let" bindings "in" expr "end" { $$ = new LetExprAST($2, $4); };

bindings:
	binding                 { $$ = {$1}; }
|	binding "," bindings    { $3.insert($3.begin(), $1); $$ = $3; };

binding:
	"id" "=" expr           { std::pair<std::string, ExprAST*> C ($1,$3); $$ = C; };

%%

void yy::parser::error (const location_type& l, const std::string& m) {
	std::cerr << l << ": " << m << '\n';
}
