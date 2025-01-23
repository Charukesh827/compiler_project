#ifndef PARSER_H
#define PARSER_H

#include <memory>
#include <vector>
#include "Token.h"
#include "AST.h"

class node
{
private:
    std::string name;
    std::vector<node*> array;

public:
    node(std::string n, std::vector<node*> a): name(n), array(a) {}
    std::string getName(){return name;}
    std::vector<node*> getArray(){return array;}
    void setName(std::string n){name = n;}
    void setArray(std::vector<node*> a){array = a;}
    void emptyArray(){array.clear();}
    int getArraySize(){return array.size();}
};


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

    //functions for parsing a exprssion
    // E -> TE'
    // E'-> +TE' | -TE' | e
    // T -> FT'
    // T'-> *FT' | /FT' | e
    // F -> (E) | id
    // Recursive Descent Parser
    

    node* E();
    node* T();
    node* Eprime();
    node* Tprime();
    node* F();

};

#endif // PARSER_H
