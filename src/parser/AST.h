#ifndef AST_H
#define AST_H

#include <string>
#include <memory>
#include <vector>

class ASTNode {
public:
    virtual ~ASTNode() = default;
};

class NumberExprAST : public ASTNode {
    double value;

public:
    explicit NumberExprAST(double val) : value(val) {}
    double getValue() const { return value; }
};

class VariableExprAST : public ASTNode {
    std::string name;
public:
    explicit VariableExprAST(const std::string &name) : name(name) {}
    const std::string getName() const { return name; }
};

class BinaryExprAST : public ASTNode {
    char op;
    std::unique_ptr<ASTNode> LHS,RHS;

public:
    explicit BinaryExprAST(char O, std::unique_ptr<ASTNode> L, std::unique_ptr<ASTNode> R ) : op(O),LHS(std::move(L)),RHS(std::move(R)) {}
    const char getOperator() const { return op; }
};

class CallExprAST : public ASTNode {
    std::string callee;
    std::vector<std::unique_ptr<ASTNode>> args;

public:
    explicit CallExprAST(const std::string &c, std::vector<std::unique_ptr<ASTNode>> args) : callee(c), args(std::move(args)) {}
    const std::string getCallee() const { return callee; }
};

class PrototypeAST : public ASTNode {
    std::string name;
    std::vector<std::string> args;
public:
    explicit PrototypeAST(const std::string &name, std::vector<std::string> args) : name(name), args(std::move(args)){}
    const std::string getName() const { return name; }
};

class FunctionAST : public ASTNode {
    std::unique_ptr<PrototypeAST> proto;
    std::vector<std::unique_ptr<ASTNode>> body;
public:
    explicit FunctionAST(std::unique_ptr<PrototypeAST> p, std::vector<std::unique_ptr<ASTNode>> b) : proto(std::move(p)), body(std::move(b)) {}    
};

class ProgramAST : public ASTNode {
    std::vector<std::unique_ptr<ASTNode>> set;
public:
    explicit ProgramAST(){}
    void addFunction(std::unique_ptr<ASTNode> x){set.push_back(x);}
};

#endif // AST_H
