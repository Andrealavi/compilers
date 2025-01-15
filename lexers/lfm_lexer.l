%{
#include <iostream>
using namespace std;
%}

ID [a-zA-Z][a-zA-Z0-9]*
INTEGER 0|[1-9][0-9]*
SEP [ \t\n]

%option noyywrap

%%

{SEP}   {}
"%"         {cout << "Operatore: " << yytext[0] << endl;}
"("         {cout << "Parentesi aperta: " << yytext[0] << endl;}
")"         {cout << "Parentesi chiusa: " << yytext[0] << endl;}
";"         {cout << "Separatore condizioni: " << yytext[0] << endl;}
":"         {cout << "Separatore condizione: " << yytext[0] << endl;}
"="         {cout << "Uguaglianza: " << yytext[0] << endl;}
","         {cout << "Separatore argomenti: " << yytext[0] << endl;}

{INTEGER}   {errno = 0; int n = stoi(yytext, NULL); cout << "Numero: " << n << endl; }

"if"        {cout << "Parola chiave: " << yytext << endl;}
"external"  {cout << "Parola chiave: " << yytext << endl;}
"function"  {cout << "Parola chiave: " << yytext << endl;}
"True"      {cout << "Parola chiave: " << yytext << endl;}
"end"       {cout << "Parola chiave: " << yytext << endl;}
{ID}        {cout << "Identificatore: " << yytext << endl;}
.           {cout << "Token sconosciuto: " << yytext[0] << endl;}

%%
