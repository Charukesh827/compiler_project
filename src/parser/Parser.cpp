#include "Parser.h"

Parser::Parser(const std::vector<Token> &tokens) : tokens(tokens) {}

// Main functions

std::unique_ptr<ASTNode> Parser::parse()
{
    std::cout<<"building parser "<<CurTok().value<<std::endl;

    std::vector<std::unique_ptr<ASTNode>> functions;
    while (matchToken(TokenType::KEYWORD) && CurTok().value == "def")
    {
        functions.push_back(FunctionParser());
    }
    return std::make_unique<ProgramAST>(std::move(functions));
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
    std::cout<<"building function "<<CurTok().value<<std::endl;

    auto proto = ProtoParser();
    getNextToken();
    auto body = BlockParser();
    return std::make_unique<FunctionAST>(std::move(proto), std::move(body));
}

std::unique_ptr<ASTNode> Parser::ProtoParser()
{
    getNextToken();
    std::cout<<"building proto "<<CurTok().value<<std::endl;
    std::string name = CurTok().value;
    std::vector<std::unique_ptr<ASTNode>> args;
    currentToken+=2;
    while (CurTok().value != ")")
    { 
        args.push_back(std::make_unique<VariableExprAST>(CurTok().value));
        getNextToken();
    }
    return std::make_unique<PrototypeAST>(name, std::move(args));
}

std::vector<std::unique_ptr<ASTNode>> Parser::BlockParser()
{
    std::vector<std::unique_ptr<ASTNode>> statements;
    getNextToken();
    std::cout<<"building block "<<CurTok().value<<std::endl;
    while (CurTok().value != "}")
    {
        if (matchToken(TokenType::KEYWORD) && CurTok().value == "if")
        {
            currentToken += 2;
            auto condition = parseExpression();
            currentToken += 2;
            auto block = BlockParser();
            statements.push_back(std::make_unique<ConditionAST>("if", std::move(condition), std::move(block)));
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
                statements.push_back(std::make_unique<ConditionAST>("elif", std::move(condition), std::move(block)));
            }
            else
            {
                std::unique_ptr<ASTNode> condition;
                auto block = BlockParser();
                statements.push_back(std::make_unique<ConditionAST>("else", std::move(condition), std::move(block)));
            }
        }
        else if (matchToken(TokenType::KEYWORD) && CurTok().value == "while")
        {
            currentToken += 2;
            auto cond = parseExpression();
            currentToken += 2;
            auto block = BlockParser();
            statements.push_back(std::make_unique<LoopAST>(std::move(cond), std::move(block)));
        }else if(matchToken(TokenType::IDENTIFIER) && tokens[currentToken+1].value=="("){
            std::string name = CurTok().value;
            currentToken+=2;
            std::vector<std::unique_ptr<ASTNode>> args;
            while(CurTok().value!=")"){
                if(matchToken(TokenType::IDENTIFIER)){
                    args.push_back(std::make_unique<VariableExprAST>(getNextToken().value));
                }else if(matchToken(TokenType::NUMBER)){
                    args.push_back(std::make_unique<NumberExprAST>(std::stod(getNextToken().value)));
                }
            }
            getNextToken();
            statements.push_back(std::make_unique<CallExprAST>(name,std::move(args)));
        }
        else
        {
            statements.push_back(parseExpression());
            getNextToken();
        }
    }
    return statements;
}

// Expression parsers

std::unique_ptr<ASTNode> Parser::parseExpression()
{
    std::cout<<"building expression "<<CurTok().value<<std::endl;

    return parseEquality();
}

std::unique_ptr<ASTNode> Parser::parseEquality()
{
    std::cout<<"building equality "<<CurTok().value<<std::endl;

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
    std::cout<<"building comparison "<<CurTok().value<<std::endl;

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
    std::cout<<"building term "<<CurTok().value<<std::endl;

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
    std::cout<<"building factor "<<CurTok().value<<std::endl;

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
    std::cout<<"building primary "<<CurTok().value<<std::endl;

    const Token &token = CurTok();

    if (token.type == TokenType::NUMBER)
    {
        getNextToken();
        return std::make_unique<NumberExprAST>(std::stod(token.value));
    }

    if (token.type == TokenType::IDENTIFIER && tokens[currentToken + 1].value == "="){
        return parseAssignment();
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
    std::cout<<"building assignment "<<CurTok().value<<std::endl;

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
