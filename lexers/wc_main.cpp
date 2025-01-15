#include <iostream>
#include <FlexLexer.h> // This must be imported in order to use the lexer defined in another file

extern unsigned long charCount, wordCount, lineCount;

int main(int argc, char **argv) {
    FlexLexer* lexer = new yyFlexLexer;
    lexer->yylex();

    std::cout << "Char Count: " << charCount << std::endl;
    std::cout << "Line Count: " << lineCount << std::endl;
    std::cout << "Word Count: " << wordCount << std::endl;

    return 0;
}
