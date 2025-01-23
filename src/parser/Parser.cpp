#include "Parser.h"
#include <stdexcept>

Parser::Parser(const std::vector<Token> &tokens) : tokens(tokens) {}

const Token &Parser::CurTok()
{
    return tokens[currentToken];
}

const Token &Parser::getNextToken()
{
    return tokens[++currentToken];
}

std::unique_ptr<ASTNode> Parser::CreateProto()
{

    // Reading the function header and creating prototype
    //   def function_name(args*)

    std::string name = getNextToken().value; // function name
    getNextToken();                          // eat '('
    std::vector<std::unique_ptr<ASTNode>> args;
    while (CurTok().value != ")") // loop until ')'
    {
        // add the arguments as an variabe type into the array
        args.push_back(std::make_unique<VariableExprAST>(CurTok().value));
        if (getNextToken().value == ",")
        {
            getNextToken();
        }
    }
    return std::make_unique<PrototypeAST>(name, args);
}

void Parser::CreateBlock(std::vector<std::unique_ptr<ASTNode>>& body)
{
    // Reading the block inside the function
    // content inside {}

    auto token=getNextToken();      // eat '{'
    while (token.value != "}")      // loop untill '}'
    {
        if (token.type == TokenType::NUMBER)
        {
            double value = std::stod(token.value);
            queue.push_back(std::make_unique<NumberExprAST>(value));
        }

        // if it is an indentifier push into queue
        if (token.type == TokenType::IDENTIFIER)
        {
            std::string name = token.value;
            queue.push_back(std::make_unique<VariableExprAST>(name));
        }

        /* parse assignment operations */
        if (token.value == "=")
        {
            body.push_back(std::make_unique<BinaryExprAST>('=', queue.front(), ExpressionParser()));
            queue.erase(queue.begin());
        }
    }
}

std::unique_ptr<ASTNode> Parser::FunctionParser()
{
    auto token = CurTok();
    if (token.type == TokenType::KEYWORD)
    {
        if (token.value == "def")
        {
            auto proto = CreateProto(); // create prototype
            std::vector<std::unique_ptr<ASTNode>> body; 
            CreateBlock(body); // create block
            return std::make_unique<FunctionAST>(proto,body);
        }
    }

    throw std::runtime_error("Unknown expression");
}

std::unique_ptr<ASTNode> Parser::parse()
{
    auto root = std::make_unique<ProgramAST>();
    while (currentToken<tokens.size())
    {
        root->addFunction(FunctionParser());
    }
    
    return root;
}
