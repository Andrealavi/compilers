#include <FlexLexer.h>

int main(int argc, char **argv) {

    FlexLexer* lexer = new yyFlexLexer;
    lexer->yylex();

    return 0;
}
