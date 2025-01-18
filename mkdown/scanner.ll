%{
#include <cerrno>
#include <cstdlib>
#include "parser.hpp"

extern text::location location;
extern bool trace_scanning;

%}

%option noyywrap nounput batch debug noinput

word [a-zA-Z0-9]*
link [a-zA-Z0-9-]+(\.[a-zA-Z0-9-]+)+
blank [ \t]
newline [\n]

%{
  // Macro definizione inserita nel codice generato da Flex
  // YY_USER_ACTION viene eseguita quando Flex riconosce un token
  # define YY_USER_ACTION  loc.columns (yyleng);
%}

%%

%{
  // Codice eseguito tutte le volte che viene invocata yylex
  text::location& loc = location;
  loc.step ();
%}

{blank}+            loc.step();
{newline}+          loc.lines(yyleng); loc.step();

"*"                 return text::parser::make_STAR(loc);
"-"                 return text::parser::make_MINUS(loc);
";"                 return text::parser::make_SEPARATOR(loc);
"["                 return text::parser::make_LSPAREN(loc);
"]"                 return text::parser::make_RSPAREN(loc);

{word}              return text::parser::make_WORD(yytext, loc);
{link}              return text::parser::make_LINK(yytext, loc);
.                   {
                        throw text::parser::syntax_error
                        (loc, "invalid character: " + std::string(yytext));
                    }
<<EOF>>    return text::parser::make_END (loc);

%%

void scan_begin(const std::string &file) {
    // This function is used to make the scanning start

    yy_flex_debug = trace_scanning;

    // If the input is - the input is read from standard input
    if (file.empty () || file == "-") {
        yyin = stdin;
    } else if (!(yyin = fopen(file.c_str (), "r"))) {
        std::cerr << "cannot open " << file << ": " << strerror(errno) << '\n';
        exit (EXIT_FAILURE);
    }
}

void scan_end() {
    fclose (yyin);
}
