#include "Lexer.h"
#include <cctype>
#include <stdexcept>
#include <iostream>
Lexer::Lexer(const std::string &source) : source(source) {}

std::vector<Token> Lexer::tokenize() {
    std::vector<Token> tokens;
    while (index < source.length()) {
        auto token = getNextToken();
        tokens.push_back(token);
        if (token.type == TokenType::EOF_TOKEN) break;
    }
    return tokens;
}

char Lexer::getNextChar() {
    if (index >= source.size()) return '\0';
    char ch = source[++index];
    if (index!=0 && source[index-1] == '\n') {
        line++;
        column = 1;
    } else {
        column++;
    }
    return ch;
}

char Lexer::getChar(){
    if (index >= source.size()) return '\0';
    char ch = source[index];
    return ch;
}

void Lexer::ungetChar() {
    if (index > 0) {
        index--;
        column--;
    }
}


Token Lexer::getNextToken() {
    char ch = getChar();
    while (isspace(ch) || ch == ',') {
        ch=getNextChar();
    }
    size_t col = column;
    std::cout<<ch<<std::endl;
    if (isalpha(ch)) {
        std::string identifier;
        while (isalnum(ch) || ch == '_') {
            identifier += ch;
            ch = getNextChar();
        }
        if (identifier == "def" || identifier == "extern" || identifier == "if" || identifier == "else" || identifier == "while")
        {
            return Token(TokenType::KEYWORD, identifier, line, col);
        }else
        {        
            return Token(TokenType::IDENTIFIER, identifier, line, col);
        }
    }

    if (isdigit(ch)) {
        std::string number;
        while (isdigit(ch) || ch == '.') {
            number += ch;
            ch = getNextChar();
        }
        return Token(TokenType::NUMBER, number, line, col);
    }

    if(ch=='=' && source[index+1] == '='){
        index+=2;
        return Token(TokenType::OPERATOR, "==", line, column);
    }

    if(ch=='>' && source[index+1] == '='){
        index+=2;
        return Token(TokenType::OPERATOR, ">=", line, column);
    }

    if(ch=='<' && source[index+1] == '='){
        index+=2;
        return Token(TokenType::OPERATOR, "<=", line, column);
    }
    if(ch=='<' && source[index+1] == '>'){
        index+=2;
        return Token(TokenType::OPERATOR, "<>", line, column);
    }

    if (ch == '+' || ch == '-' || ch == '*' || ch == '/' || ch == '=' || ch == '(' || ch == ')' || ch == '<' || ch == '>' || ch == '!' || ch == '{' || ch == '}') 
    {
        getNextChar();

        return Token(TokenType::OPERATOR, std::string(1,ch), line, col);
    }

    if(ch == ';'){
        getNextChar();
        return Token(TokenType::EOL_TOKEN, std::string(1,ch),line,col);
    }

    if (ch == '\0') {
        return Token(TokenType::EOF_TOKEN, "EOF", line, col);
    }

    return Token(TokenType::INVALID, std::string(1, ch), line, col);
}
