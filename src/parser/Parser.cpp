#include "Parser.h"

Parser::Parser(const std::vector<Token> &tokens) : tokens(tokens) {}

// Main functions

std::unique_ptr<ASTNode> Parser::parse()
{
    std::vector<std::unique_ptr<ASTNode>> functions;
    while (matchToken(TokenType::KEYWORD) && CurTok().value == "def")
    {
        functions.push_back(FunctionParser());
    }
    return parseExpression();
}

// Helper functions

const Token &Parser::CurTok()
{
    return tokens[currentToken];
}

const Token &Parser::getNextToken()
{
    return tokens[currentToken++];
}

bool Parser::matchToken(TokenType type)
{
    if (CurTok().type == type)
    {
        return true;
    }
    return false;
}

// Parser fuctions

std::unique_ptr<ASTNode> Parser::FunctionParser()
{
    auto proto = ProtoParser();
    auto body = BlockParser();
    return std::make_unique<FunctionAST>(proto, body);
}

std::unique_ptr<ASTNode> Parser::ProtoParser()
{
    getNextToken();
    auto name = CurTok();
    std::vector<std::unique_ptr<ASTNode>> args;
    getNextToken();
    while (CurTok().value != ")")
    {
        args.push_back(std::make_unique<VariableExprAST>(CurTok().value));
    }
    return std::make_unique<PrototypeAST>(name, args);
}

std::unique_ptr<ASTNode> Parser::BlockParser()
{
    std::vector<std::unique_ptr<ASTNode>> statements;
    getNextToken();
    while (CurTok().value != "}")
    {
        if (matchToken(TokenType::KEYWORD) && CurTok().value == "if")
        {
            currentToken += 2;
            auto condition = parseExpression();
            currentToken += 2;
            auto block = BlockParser();
            statements.push_back(std::make_unique<ConditionAST>("if", condition, block));
        }
        else if (matchToken(TokenType::KEYWORD) && CurTok().value == "else")
        {
            getNextToken();
            if (CurTok().value == "if")
            {
                currentToken += 2;
                auto condition = parseExpression();
                currentToken += 2;
                auto block = BlockParser();
                statements.push_back(std::make_unique<ConditionAST>("elif", condition, block));
            }
            else
            {
                std::unique_ptr<ASTNode> condition;
                auto block = BlockParser();
                statements.push_back(std::make_unique<ConditionAST>("else", condition, block));
            }
        }
        else if (matchToken(TokenType::KEYWORD) && CurTok().value == "while")
        {
            currentToken += 2;
            auto cond = parseExpression();
            currentToken += 2;
            auto block = BlockParser();
            statements.push_back(std::make_unique<LoopAST>(cond, block));
        }
        else
        {
            statements.push_back(parseExpression());
            getNextToken();
        }
    }
}

// Expression parsers

std::unique_ptr<ASTNode> Parser::parseExpression()
{
    return parseEquality();
}

std::unique_ptr<ASTNode> Parser::parseEquality()
{
    auto lhs = parseComparison();

    while (matchToken(TokenType::OPERATOR) &&
           (CurTok().value == "==" || CurTok().value == "<>"))
    {
        std::string op = getNextToken().value;
        auto rhs = parseComparison();
        lhs = std::make_unique<BinaryExprAST>(op, std::move(lhs), std::move(rhs));
    }

    return lhs;
}

std::unique_ptr<ASTNode> Parser::parseComparison()
{
    auto lhs = parseTerm();

    while (matchToken(TokenType::OPERATOR) &&
           (CurTok().value == "<" || CurTok().value == ">" ||
            CurTok().value == "<=" || CurTok().value == ">="))
    {
        std::string op = getNextToken().value;
        auto rhs = parseTerm();
        lhs = std::make_unique<BinaryExprAST>(op, std::move(lhs), std::move(rhs));
    }

    return lhs;
}

std::unique_ptr<ASTNode> Parser::parseTerm()
{
    auto lhs = parseFactor();

    while (matchToken(TokenType::OPERATOR) &&
           (CurTok().value == "+" || CurTok().value == "-"))
    {
        std::string op = getNextToken().value;
        auto rhs = parseFactor();
        lhs = std::make_unique<BinaryExprAST>(op, std::move(lhs), std::move(rhs));
    }

    return lhs;
}

std::unique_ptr<ASTNode> Parser::parseFactor()
{
    auto lhs = parsePrimary();

    while (matchToken(TokenType::OPERATOR) &&
           (CurTok().value == "*" || CurTok().value == "/"))
    {
        std::string op = getNextToken().value;
        auto rhs = parsePrimary();
        lhs = std::make_unique<BinaryExprAST>(op, std::move(lhs), std::move(rhs));
    }

    return lhs;
}

std::unique_ptr<ASTNode> Parser::parsePrimary()
{
    const Token &token = CurTok();

    if (token.type == TokenType::NUMBER)
    {
        getNextToken();
        return std::make_unique<NumberExprAST>(std::stod(token.value));
    }

    if (token.type == TokenType::IDENTIFIER)
    {
        getNextToken();
        return std::make_unique<VariableExprAST>(token.value);
    }

    if (matchToken(TokenType::PUNCTUATION) && token.value == "(")
    {
        auto expr = parseExpression();
        if (!matchToken(TokenType::PUNCTUATION) || CurTok().value != ")")
        {
            throw std::runtime_error("Expected closing parenthesis");
        }
        getNextToken();
        return expr;
    }

    throw std::runtime_error("Unexpected token: " + token.value);
}

std::unique_ptr<ASTNode> Parser::parseAssignment()
{
    const Token &token = CurTok();

    if (token.type == TokenType::IDENTIFIER && tokens[currentToken + 1].value == "=")
    {
        std::string varName = token.value;
        getNextToken(); // Consume identifier
        getNextToken(); // Consume '='
        auto value = parseExpression();
        return std::make_unique<BinaryExprAST>("=", std::make_unique<VariableExprAST>(varName), std::move(value));
    }

    return parseEquality();
}
