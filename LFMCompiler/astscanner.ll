%{ /* -*- C++ -*- */
# include <cerrno>
# include <climits>
# include <cstdlib>
# include <string>
# include <cmath>
# include "astparser.hpp"
# include "astdriver.hpp"
%}

/*
    noyywrap: Prevents scanner to call yywrap() at the end of the file/input stream. Yywrap is used when input is on multiple files
    nounput: Omits unput() function from the scanner generated C++ code. Unput function is used to place characters back on the input stream in order to
        re-read them
    batch: Tells the scanner to operate using batches. It makes possible to have a slight gain in performance when the scanner is not used interactively
    debug: Makes the scanner run in debug mode
    noinput: Omits input() function from the scanner generated code. Input function is used to take characters from the input stream, therefore it should be
        reimplemented. In this file, it is substituted with the scan_begin function from the driver
*/
%option noyywrap nounput batch debug noinput

id      [a-zA-Z_][a-zA-Z_0-9]*
num     0|[1-9][0-9]*
blank   [ \t]

%{
    // This code is executed each time there is a match with a regex
    # define YY_USER_ACTION loc.columns(yyleng);
%}

%%

%{
    // The location is stored in the driver, but it is useful
    // to be able to reference it more succinctly
    // with a local variable
    yy::location& loc = drv.location;

    // Executed each time yylex() is called
    loc.step ();
%}

{blank}+   loc.step ();
[\n]+      loc.lines (yyleng); loc.step ();

":"      return yy::parser::make_COLON     (loc);
";"      return yy::parser::make_SEMICOLON (loc);
","      return yy::parser::make_COMMA     (loc);
"-"      return yy::parser::make_MINUS     (loc);
"+"      return yy::parser::make_PLUS      (loc);
"*"      return yy::parser::make_STAR      (loc);
"/"      return yy::parser::make_SLASH     (loc);
"%"      return yy::parser::make_MOD       (loc);
"("      return yy::parser::make_LPAREN    (loc);
")"      return yy::parser::make_RPAREN    (loc);
"|"      return yy::parser::make_ALT       (loc);
"<="     return yy::parser::make_LE        (loc);
"<>"     return yy::parser::make_NEQ       (loc);
"<"      return yy::parser::make_LT        (loc);
"=="     return yy::parser::make_EQ        (loc);
">="     return yy::parser::make_GE        (loc);
">"      return yy::parser::make_GT        (loc);
"="      return yy::parser::make_BIND      (loc);
"true"   return yy::parser::make_TRUE      (loc);
"false"  return yy::parser::make_FALSE     (loc);
"external" return yy::parser::make_EXTERN  (loc);
"function" return yy::parser::make_DEF     (loc);
"if"     return yy::parser::make_IF        (loc);
"and"    return yy::parser::make_AND       (loc);
"or"     return yy::parser::make_OR        (loc);
"not"    return yy::parser::make_NOT       (loc);
"let"    return yy::parser::make_LET       (loc);
"in"     return yy::parser::make_IN        (loc);
"end"    return yy::parser::make_END       (loc);

{num}   {
    errno = 0;
    int n;

    /*
        If the number can't be converted from string to int throws an error
    */
    try {
        n = std::stoi(yytext, NULL, 10);
    } catch(std::out_of_range& e) {
        std::cerr << "Out of Range error: " << e.what() << '\n';
        throw yy::parser::syntax_error(loc, "Integer number too big: "
            + std::string(yytext));
    };

    return yy::parser::make_NUMBER(n, loc);
}

{id}     { return yy::parser::make_IDENTIFIER(yytext, loc); }

.        { throw yy::parser::syntax_error(loc, "invalid character: " + std::string(yytext)); }

<<EOF>>  { return yy::parser::make_EOF(loc); }

%%

/*
    Substitutes input() function.
    If one of the argument is '-', the scanner sets standard input as the default input, otherwise it opens the given file.
*/
void driver::scan_begin () {
    yy_flex_debug = trace_scanning;

    if (file.empty () || file == "-") {
        yyin = stdin;
    } else if (!(yyin = fopen (file.c_str (), "r"))) {
        std::cerr << "cannot open " << file << ": " << strerror(errno) << '\n';
        exit (EXIT_FAILURE);
    }
}

/*
    Closes the input stream
*/
void driver::scan_end() {
    fclose(yyin);
}
