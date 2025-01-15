/*
This lexer recreates the wc command
*/

%{
#include <iostream>
using namespace std;

unsigned long charCount = 0, wordCount = 0, lineCount = 0;
%}

word [^ \t\n]
eol \n

%option noyywrap

%%

{word}    {wordCount++; charCount += yyleng;}
{eol}     {lineCount++;}
.         charCount++;

%%
