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
  class LoopExprAST;

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
  LSPAREN    "["
  RSPAREN    "]"
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
  TERNARY    "?"
  ARRAY      "array"
  TRUE       "true"
  FALSE      "false"
  EXTERN     "external"
  FORWARD    "forward"
  DEF        "function"
  GLOBAL     "global"
  CONST      "const"
  FOR        "for"
  RANGE      "range"
  DO         "do"
  WHILE      "while"
  BREAK      "break"
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
%type <PrototypeAST*> forwarddef
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
%type <ExprAST*> ternaryexpr
%type <LetExprAST*> letexpr
%type <ExprAST*> literal
%type <ExprAST*> relexpr
%type <ExprAST*> identifier
%type <ExprAST*> arraydef
%type <ExprAST*> arraycomprehension
%type <ExprAST*> assignment
%type <ExprAST*> var_or_array
%type <LoopExprAST*> loopexpr
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
|   forwarddef            { $$ = $1; }
|   funcdef               { $$ = $1; }
|   globdef               { $$ = $1; };

extdef:
    "external" prototype  { $2->setext(); $$ = $2; };

forwarddef:
    "forward"  prototype  { $2->setfor(); $$ = $2; };

funcdef:
    "function" prototype exprs "end"  { $$ = new FunctionAST($2,$3); };

prototype:
    "id" "(" params ")"   { $$ = new PrototypeAST($1,$3); };

params:
    %empty                { std::vector<std::string> params; $$ = params; }
|   "id" params           { $2.insert($2.begin(),$1); $$ = $2;};

%nonassoc "<" "==" "<>" "<=" ">" ">=" "?" ":";
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
|   assignment                          { $$ = $1; }
|   arraydef                            { $$ = $1; }
|   "const" binding                     { $$ = new AssignmentExprAST($2, true); }
|   retexpr                             { $$ = $1; };

assignment:
    binding                             { $$ = new AssignmentExprAST($1); }
|   "id" "[" "number" "]" "=" expr      { std::pair<std::string, ExprAST*> C ($1,$6); $$ = new AssignmentExprAST(C, $3); };

identifier:
    "id"                   { $$ = new IdeExprAST($1); }

var_or_array:
    identifier             { $$ = $1; }
|   "id" "[" "number" "]"  { $$ = new IdeExprAST($1, $3); };

expr:
    expr "+" expr          { $$ = new BinaryExprAST("+",$1,$3); }
|   expr "-" expr          { $$ = new BinaryExprAST("-",$1,$3); }
|   expr "*" expr          { $$ = new BinaryExprAST("*",$1,$3); }
|   expr "/" expr          { $$ = new BinaryExprAST("/",$1,$3); }
|   expr "^" expr          { $$ = new ExponentiationExprAST($1, $3); }
|   expr "%" expr          { $$ = new BinaryExprAST("%",$1,$3); }
|   "-" expr %prec UMINUS  { $$ = new UnaryExprAST("-",$2); }
|   "(" expr ")"           { $$ = $2; }
|   var_or_array           { $$ = $1; }
|   "number"               { $$ = new NumberExprAST($1); }
|   "break"                { $$ = new BreakExprAST(); }
|   condexpr               { $$ = $1; }
|   pipexpr                { $$ = new PipExprAST($1); }
|   loopexpr               { $$ = $1; }
|   ternaryexpr            { $$ = $1; }
|   letexpr                { $$ = $1; };


arglist:
    %empty                 { std::vector<ExprAST*> args; $$ = args; }
|   args                   { $$ = $1; };

args:
    expr                   { std::vector<ExprAST*> V = {$1}; $$ = V; }
|   expr "," args          { $3.insert($3.begin(),$1); $$ = $3; };

condexpr:
    "if" pairs "end"            { $$ = new IfExprAST($2); }

ternaryexpr:
   boolexpr "?" expr ":" expr  { $$ = new TernaryExprAST($1, $3, $5); };

pipexpr:
    callexpr "|>" pipexpr  { $3.insert($3.begin(), $1); $$ = $3; }
|   callexpr               { std::vector<ExprAST*> V = {$1}; $$ = V; };

callexpr:
    "id" "(" arglist ")"   { $$ = new CallExprAST($1, $3); };

loopexpr:
    "for" "(" binding ";" boolexpr ";" expr ")" exprs "end"     { $$ = new ForExprAST($3, $5, $7, $9); }
|   "do" "{" exprs "}" "while" "(" boolexpr ")" "end"           { $$ = new DoWhileExprAST($7, $3); }
|   "for" "(" identifier ":" identifier ")" exprs "end"     { $$ = new ForRangeExprAST($3, $5, $7); }

arraycomprehension:
    "{" expr "for" "id" "in" "range" "(" "number" ")" "}"       {
                                                                    ExprAST* numberExpr = new NumberExprAST(0);
                                                                    std::pair<std::string, ExprAST*> C ($4, numberExpr);

                                                                    ExprAST* counterExpr = new IdeExprAST($4);
                                                                    BinaryExprAST* booleanExpression = new BinaryExprAST("<",counterExpr, new NumberExprAST($8));
                                                                    BinaryExprAST* updateExpression = new BinaryExprAST("+", counterExpr, new NumberExprAST(1));

                                                                    $$ = new ComprExprAST(C, booleanExpression, updateExpression, $2);
                                                                };

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
    "id" "=" expr           { std::pair<std::string, ExprAST*> C ($1,$3); $$ = C; }

arraydef:
    "array" "id" "=" "{" args "}"       { $$ = new ArrayExprAST($2, $5); };
|   "array" "id" "=" arraycomprehension { $$ = new ArrayExprAST($2, $4); }

%%

void yy::parser::error (const location_type& l, const std::string& m) {
    std::cerr << l << ": " << m << '\n';
}
