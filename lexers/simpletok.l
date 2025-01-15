/*
This file defines a simple tokenizer that should match + - and every number, otherwise it prints that the token is unknown
*/

%{
#include <iostream>
using namespace std;
%}

DIGIT [0-9]
DIGIT1 [1-9]

%option noyywrap

%%

"+"                 {cout << "  operator <" << yytext[0] << ">" << endl; }
"-"                 {cout << "  operator <" << yytext[0] << ">" << endl; }
"="                 {cout << "  operator <" << yytext[0] << ">" << endl; }
{DIGIT1}{DIGIT}*    {cout << "  number <" << yytext << ">" << endl; }
.                   {cout << "Unknown token" << endl; }

%%

int main(int argc, char **argv) {
    FlexLexer* lexer = new yyFlexLexer;
    lexer->yylex();

    return 0;
}
