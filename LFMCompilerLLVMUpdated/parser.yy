%skeleton "lalr1.cc" /* -*- C++ -*- */
%require "3.2"
%defines

%define api.token.constructor
%define api.location.file none
%define api.value.type variant
%define parse.assert
%define parse.trace
%define parse.error verbose

%code requires {
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

%param { driver& drv }

%locations

%code {
# include "driver.hpp"
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
  EXP        "^"
  SLASH      "/"
  MOD        "%"
  LPAREN     "("
  RPAREN     ")"
  LCPAREN    "{"
  RCPAREN    "}"
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
  GLOBAL     "global"
  FOR        "for"
  RETURN     "return"
  IF         "if"
  PIPE       "|>"
  AND        "and"
  OR         "or"
  NOT        "not"
  LET        "let"
  IN         "in"
  END        "end"
;

%token <std::string> IDENTIFIER "id"
%token <int> NUMBER "number"

%type <std::vector<DefAST*>> deflist
%type <DefAST*> def
%type <FunctionAST*> funcdef
%type <PrototypeAST*> extdef
%type <PrototypeAST*> prototype
%type <std::vector<std::string>> params
%type <std::vector<ExprAST*>> arglist
%type <std::vector<ExprAST*>> args
%type <DefAST*> globdef

%type <std::vector<ExprAST*>> exprs
%type <std::vector<ExprAST*>> exprs_list
%type <ExprAST*> expr_or_other

%type <ExprAST*> expr
%type <ExprAST*> boolexpr
%type <IfExprAST*> condexpr
%type <LetExprAST*> letexpr
%type <ExprAST*> literal
%type <ExprAST*> relexpr
%type <ExprAST*> forexpr
%type <ExprAST*> retexpr
%type <ExprAST*> callexpr

%type <std::vector<std::pair<ExprAST*, std::vector<ExprAST*>>>> pairs;
%type <std::pair<ExprAST*, std::vector<ExprAST*>>> pair;
%type <std::vector<std::pair<std::string, ExprAST*>>> bindings;
%type <std::pair<std::string, ExprAST*>> binding;
%type <std::vector<ExprAST*>> pipexpr

%%

%start startsymb;

startsymb:
    deflist               { drv.root = $1;};

deflist:
    def deflist           { $2.insert($2.begin(),$1); $$ = $2; }
|   def                   { std::vector<DefAST*> D = {$1}; $$ = D; };

def:
    extdef                { $$ = $1; }
|   funcdef               { $$ = $1; }
|   globdef               { $$ = $1; };

extdef:
    "external" prototype  { $2->setext(); $$ = $2; };

funcdef:
    "function" prototype exprs "end"  { $$ = new FunctionAST($2,$3); };

prototype:
    "id" "(" params ")"   { $$ = new PrototypeAST($1,$3); };

params:
    %empty                { std::vector<std::string> params; $$ = params; }
|   "id" params           { $2.insert($2.begin(),$1); $$ = $2;};

%nonassoc "<" "==" "<>" "<=" ">" ">=";
%left "+" "-";
%left "*" "/" "%";
%right "^";
%nonassoc UMINUS;
%left "or";
%left "and";
%nonassoc NEGATE;

exprs:
    exprs_list           { $$ = $1; }
|   %empty               { std::vector<ExprAST*> V = {}; $$ = V; };

exprs_list:
    expr_or_other                     { $$ = std::vector<ExprAST*>{$1}; }
|   expr_or_other ";" exprs_list      { $3.insert($3.begin(), $1); $$ = $3; };

expr_or_other:
    expr                                { $$ = $1; }
|   binding                             { $$ = new AssignmentExprAST($1);; }
|   retexpr                             { $$ = $1; };

expr:
    expr "+" expr          { $$ = new BinaryExprAST("+",$1,$3); }
|   expr "-" expr          { $$ = new BinaryExprAST("-",$1,$3); }
|   expr "*" expr          { $$ = new BinaryExprAST("*",$1,$3); }
|   expr "/" expr          { $$ = new BinaryExprAST("/",$1,$3); }
|   expr "^" expr          { $$ = new ExponentiationExprAST($1, $3); }
|   expr "%" expr          { $$ = new BinaryExprAST("%",$1,$3); }
|   "-" expr %prec UMINUS  { $$ = new UnaryExprAST("-",$2); }
|   "(" expr ")"           { $$ = $2; }
|   "id"                   { $$ = new IdeExprAST($1); }
|   "number"               { $$ = new NumberExprAST($1); }
|   condexpr               { $$ = $1; }
|   pipexpr                { $$ = new PipExprAST($1); }
|   forexpr                { $$ = $1; }
|   letexpr                { $$ = $1; };


arglist:
    %empty                 { std::vector<ExprAST*> args; $$ = args; }
|   args                   { $$ = $1; };

args:
    expr                   { std::vector<ExprAST*> V = {$1}; $$ = V; }
|   expr "," args          { $3.insert($3.begin(),$1); $$ = $3; };

condexpr:
    "if" pairs "end"       { $$ = new IfExprAST($2); };

pipexpr:
    callexpr "|>" pipexpr  { $3.insert($3.begin(), $1); $$ = $3; }
|   callexpr               { std::vector<ExprAST*> V = {$1}; $$ = V; };

callexpr:
    "id" "(" arglist ")"   { $$ = new CallExprAST($1, $3); };

forexpr:
    "for" "(" binding ";" boolexpr ";" expr ")" exprs "end"   { $$ = new ForExprAST($3, $5, $7, $9); };

pairs:
    pair                   { std::vector<std::pair<ExprAST*, std::vector<ExprAST*>>> P = {$1}; $$ = P; }
|   pair pairs             { $2.insert($2.begin(),$1); $$ = $2; };

pair:
    boolexpr "{" exprs "}"     { std::pair<ExprAST*,std::vector<ExprAST*>> P ($1,$3); $$ = P; };

boolexpr:
    boolexpr "and" boolexpr { $$ = new BinaryExprAST("and",$1,$3); }
|   boolexpr "or" boolexpr  { $$ = new BinaryExprAST("or",$1,$3); }
|   "not" boolexpr  %prec NEGATE { $$ = new UnaryExprAST("not",$2); }
|   literal                 { $$ = $1; }
|   relexpr                 { $$ = $1; };

retexpr:
    "return" expr           { $$ = new RetExprAST($2); };

literal:
    "true"                  { $$ = new BoolConstAST(1); }
|   "false"                 { $$ = new BoolConstAST(0); };

relexpr:
    expr "<"  expr          { $$ = new BinaryExprAST("<",$1,$3); }
|   expr "==" expr          { $$ = new BinaryExprAST("==",$1,$3); }
|   expr "<>" expr          { $$ = new BinaryExprAST("<>",$1,$3); }
|   expr "<=" expr          { $$ = new BinaryExprAST("<=",$1,$3); }
|   expr ">"  expr          { $$ = new BinaryExprAST(">",$1,$3); }
|   expr ">=" expr          { $$ = new BinaryExprAST(">=",$1,$3); }

letexpr:
    "let" bindings "in" exprs "end" { $$ = new LetExprAST($2,$4); };

globdef:
    "global" "id"           { $$ = new GlobalDefAST($2); }
|   "global" "id" "=" expr        { $$ = new GlobalDefAST($2, $4); };

bindings:
    binding                 { std::vector<std::pair<std::string, ExprAST*>> B = {$1}; $$ = B; }
|   binding "," bindings    { $3.insert($3.begin(),$1); $$ = $3; };

binding:
    "id" "=" expr           { std::pair<std::string, ExprAST*> C ($1,$3); $$ = C; };

%%

void yy::parser::error (const location_type& l, const std::string& m) {
    std::cerr << l << ": " << m << '\n';
}
