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

// Defining Expression parser recurson functions

node *Parser::E()
{
    std::vector<node *> v;
    v.push_back(T());
    v.push_back(Eprime());
    return new node("E", v);
}

node *Parser::Eprime()
{
    std::vector<node *> v;
    if (CurTok().value == "+")
    {
        v.push_back(new node("+", v));
        getNextToken();
        v.push_back(T());
        v.push_back(Eprime());
        return new node("E'", v);
    }
    else if (CurTok().value == "-")
    {
        v.push_back(new node("-", v));
        getNextToken();
        v.push_back(T());
        v.push_back(Eprime());
        return new node("E'", v);
    }
    else
    {
        return new node("e", v);
    }
}

node *Parser::T()
{
    std::vector<node *> v;
    v.push_back(F());
    v.push_back(Tprime());
    return new node("T", v);
}

node *Parser::Tprime()
{
    std::vector<node *> v;
    if (CurTok().value == "*")
    {
        v.push_back(new node("*", v));
        getNextToken();
        v.push_back(F());
        v.push_back(Tprime());
        return new node("T'", v);
    }
    else if (CurTok().value == "/")
    {
        v.push_back(new node("/", v));
        getNextToken();
        v.push_back(F());
        v.push_back(Tprime());
        return new node("T'", v);
    }
    else
    {
        return new node("e", v);
    }
}

node *Parser::F()
{
    std::vector<node *> v;
    if (CurTok().value == "(")
    {
        v.push_back(new node("(", v));
        getNextToken();
        v.push_back(E());
        if (CurTok().value == ")")
        {
            v.push_back(new node(")", v));
            getNextToken();
            return new node("F", v);
        }
        else
        {
            // error("Missing closing parenthesis");
            exit(23);
        }
    }
    else if (CurTok().type == TokenType::IDENTIFIER)
    {
        auto name = CurTok().value;
        getNextToken();
        return new node(name, v);
    }
    else if (CurTok().type == TokenType::NUMBER)
    {
        auto name = std::string(CurTok().value);
        getNextToken();
        return new node(name, v);
    }
    else
    {
        // error("Invalid token");
        exit(24);
    }
}

std::unique_ptr<ASTNode> Parser::ExpressionParser()
{
    auto root = E();
    std::vector<node *> stack, help;
    stack.push_back(root);
    node* hold;
    node* big;
    std::string name, child;
    int sizebefore=1;
    while (stack.size() > 0)
    {
        hold = stack.back();
        name = hold->getName();
        if (name == "E" || name == "E'" || name == "T" || name == "T'" || name == "F" || name == "F'")
        {
            help = hold->getArray();
            for (auto it = help.size() - 1; it > 0; --it)
            {
                child = help.at(it)->getName();
                if (child == "E" || child == "E'" || child == "T" || child == "T'" || child == "F" || child == "F'")
                {
                    stack.push_back(help.at(it));
                    //find the biggest of all
                }
                if(stack.size()==sizebefore){
                    //applythe biggest and clear the child array\
                    //remove from the stack
                }
            }
        }
        else
        {
            stack.pop_back();
        }
    }
}

// end of Expression define

void Parser::CreateBlock(std::vector<std::unique_ptr<ASTNode>> &body)
{
    // Reading the block inside the function
    // content inside {}

    auto token = getNextToken(); // eat '{'
    while (token.value != "}")   // loop untill '}'
    {
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
            return std::make_unique<FunctionAST>(proto, body);
        }
    }

    throw std::runtime_error("Unknown expression");
}

std::unique_ptr<ASTNode> Parser::parse()
{
    auto root = std::make_unique<ProgramAST>();
    while (currentToken < tokens.size())
    {
        root->addFunction(FunctionParser());
    }

    return root;
}
