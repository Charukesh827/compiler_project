#ifndef PARSER_H
#define PARSER_H

#include <memory>
#include <vector>
#include "Token.h"
#include "AST.h"

class Parser {
public:
    explicit Parser(const std::vector<Token> &tokens);
    std::unique_ptr<ASTNode> parse();

private:
    const std::vector<Token> &tokens;
    size_t currentToken = 0;
    std::vector<std::unique_ptr<ASTNode>> queue;

    const Token& CurTok();
    const Token& getNextToken();
    std::unique_ptr<ASTNode> FunctionParser();
    std::unique_ptr<ASTNode> ExpressionParser();
    std::unique_ptr<ASTNode> CreateProto();
    void CreateBlock(std::vector<std::unique_ptr<ASTNode>>& body);
};

#endif // PARSER_H
