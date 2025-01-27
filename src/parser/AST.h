#ifndef AST_H
#define AST_H

#include <string>
#include <memory>
#include <vector>
#include <iostream>

class ASTNode
{
public:
    // virtual ~ASTNode() = default;
    virtual std::string type() = 0;
    
};

class NumberExprAST : public ASTNode
{
    double value;

public:
    explicit NumberExprAST(double val) : value(val) {}
    double getValue() const { return value; }
    std::string type()override{ return "Number";}
};

class VariableExprAST : public ASTNode
{
    std::string name;

public:
    explicit VariableExprAST(const std::string &name) : name(name) {}
    const std::string& getName() const { return name; }
    std::string type()override{ return "Variable";}
};

class BinaryExprAST : public ASTNode
{
    std::string op;
    std::unique_ptr<ASTNode> LHS, RHS;


public:
    explicit BinaryExprAST(std::string O, std::unique_ptr<ASTNode> L, std::unique_ptr<ASTNode> R) : op(O), LHS(std::move(L)), RHS(std::move(R)) {}
    const std::string& getOp() const { return op; }
    std::unique_ptr<ASTNode>& getLHS() { return LHS; }
    std::unique_ptr<ASTNode>& getRHS() { return RHS; }
    std::string type()override{ return "Operator";}
};

class ConditionAST : public ASTNode
{
    std::string Ctype;
    std::unique_ptr<ASTNode> cond;
    std::vector<std::unique_ptr<ASTNode>> block;

public:
    explicit ConditionAST(std::string t, std::unique_ptr<ASTNode> cond, std::vector<std::unique_ptr<ASTNode>> block) : Ctype(t), cond(std::move(cond)), block(std::move(block)) {}
    const std::string& getType() const { return Ctype; }
    std::unique_ptr<ASTNode>& getCond() { return cond; }
    std::vector<std::unique_ptr<ASTNode>>& getBlock() { return block; }
    std::string type()override{ return Ctype;}
};

class LoopAST : public ASTNode
{
    std::unique_ptr<ASTNode> cond;
    std::vector<std::unique_ptr<ASTNode>> block;

public:
    explicit LoopAST(std::unique_ptr<ASTNode> cond, std::vector<std::unique_ptr<ASTNode>> block) : cond(std::move(cond)), block(std::move(block)) {}
    std::unique_ptr<ASTNode>& getCond() { return cond; }
    std::vector<std::unique_ptr<ASTNode>>& getBlock() { return block; }
    std::string type()override{ return "While";}
};

class CallExprAST : public ASTNode
{
    std::string callee;
    std::vector<std::unique_ptr<ASTNode>> args;

public:
    explicit CallExprAST(const std::string &c, std::vector<std::unique_ptr<ASTNode>> args) : callee(c), args(std::move(args)) {}
    const std::string& getCallee() const { return callee; }
    std::vector<std::unique_ptr<ASTNode>>& getArgs() { return args; }
    std::string type()override{ return "Call";}
};

class PrototypeAST : public ASTNode
{
    std::string name;
    std::vector<std::unique_ptr<ASTNode>> args;

public:
    explicit PrototypeAST(const std::string &name, std::vector<std::unique_ptr<ASTNode>> args) : name(name), args(std::move(args)) {}
    const std::string& getName() const { return name; }
    std::vector<std::unique_ptr<ASTNode>>& getArgs() { return args; }
    std::string type()override{ return "Prototype";}
};

class FunctionAST : public ASTNode
{
    std::unique_ptr<ASTNode> proto;
    std::vector<std::unique_ptr<ASTNode>> body;

public:
    explicit FunctionAST(std::unique_ptr<ASTNode> p, std::vector<std::unique_ptr<ASTNode>> b) : proto(std::move(p)), body(std::move(b)) {}
    std::unique_ptr<ASTNode>& getProto() { return proto; }
    std::vector<std::unique_ptr<ASTNode>>& getBody() { return body; }
    std::string type()override{ return "Function";}
};

class ProgramAST : public ASTNode
{
    std::vector<std::unique_ptr<ASTNode>> functions;

public:
    explicit ProgramAST(std::vector<std::unique_ptr<ASTNode>> f): functions(std::move(f)) {}
    void addFunction(std::unique_ptr<ASTNode> x) { functions.push_back(std::move(x)); }
    std::vector<std::unique_ptr<ASTNode>>& getFunctions() { return functions; }
    std::string type()override{ return "Program";}
};

#endif // AST_H
