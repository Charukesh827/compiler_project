#ifndef LEXER_H
#define LEXER_H

#include <string>
#include <vector>
#include "Token.h"

class Lexer {
public:
    explicit Lexer(const std::string &source);
    std::vector<Token> tokenize(); // Tokenizes the input

private:
    std::string source;
    size_t index = 0;
    int line = 1;
    int column = 1;

    char getNextChar();
    void ungetChar();
    Token getNextToken();
    char getChar();
};

#endif // LEXER_H
