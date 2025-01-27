#ifndef PARSER_H
#define PARSER_H

#include <vector>
#include <iostream>
#include "Token.h"
#include "AST.h"

class Parser
{
public:
    explicit Parser(const std::vector<Token> &tokens);
    std::unique_ptr<ASTNode> parse();

private:
    const std::vector<Token> &tokens;
    size_t currentToken = 0;

    // helper functions

    const Token &CurTok();
    const Token &getNextToken();
    bool matchToken(TokenType type);

    // parser functions
    std::unique_ptr<ASTNode> FunctionParser();
    std::unique_ptr<ASTNode> ProtoParser();
    std::vector<std::unique_ptr<ASTNode>> BlockParser();

    // functions for parsing a exprssion
    // expression       ::= equality
    // equality         ::= comparison (("==" | "<>") comparison)*
    // comparison       ::= term (("<" | ">" | "<=" | ">=") term)*
    // term             ::= factor (("+" | "-") factor)*
    // factor           ::= primary (("*" | "/") primary)*
    // primary          ::= NUMBER | IDENTIFIER | "(" expression ")" | assignment
    // assignment       ::= IDENTIFIER "=" expression

    std::unique_ptr<ASTNode> parseExpression();
    std::unique_ptr<ASTNode> parseEquality();
    std::unique_ptr<ASTNode> parseComparison();
    std::unique_ptr<ASTNode> parseTerm();
    std::unique_ptr<ASTNode> parseFactor();
    std::unique_ptr<ASTNode> parsePrimary();
    std::unique_ptr<ASTNode> parseAssignment();
};

#endif // PARSER_H
