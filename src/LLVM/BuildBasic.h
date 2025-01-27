#ifndef BUILDBASIC_H
#define BUILDBASIC_H

#include "AST.h"
#include "llvm/IR/LLVMContext.h"
#include "llvm/IR/Module.h"
#include "llvm/IR/IRBuilder.h"
#include "llvm/IR/Verifier.h"
#include "llvm/IR/Function.h"
#include "llvm/IR/BasicBlock.h"
#include <map>

class Basic
{
public:
    Basic(std::unique_ptr<ASTNode> r) : root(std::move(r)), context(), module(std::make_unique<llvm::Module>("compiler", context)), builder(context)
    {
        current = std::string("entry");
        ifCount = 0;
        whileCount = 0;
        entryCount = 0;
        variables = std::vector<std::string>();
        table = std::map<std::string, llvm::AllocaInst *>();
        count = std::map<std::string, int>();
    }
    void print();
    llvm::Module *GenerateLLVM();

private:
    std::unique_ptr<ASTNode> root;
    llvm::LLVMContext context;            // Manages all LLVM-related states
    std::unique_ptr<llvm::Module> module; // Represents the entire LLVM IR code
    llvm::IRBuilder<> builder;            // Helps create IR instructions

    // helper variables:
    std::string current;
    int ifCount, whileCount, entryCount;
    llvm::Function *function;

    // variable keepers
    std::vector<std::string> variables;
    std::map<std::string, llvm::AllocaInst *> table;
    std::map<std::string, int> count;

    // Functions for traversing the AST nodes and generating LLVM IR

    llvm::Function *GenerateFunction(FunctionAST *node); // Updated signature
    void ControllerBasicBlock(llvm::BasicBlock *curBlock, std::vector<std::unique_ptr<ASTNode>>* block ,size_t pc);
    llvm::Value *GenerateExpression(ASTNode *node, bool left);
    llvm::AllocaInst * GetName(std::string name, bool left);
};

#endif // BUILDBASIC_H
