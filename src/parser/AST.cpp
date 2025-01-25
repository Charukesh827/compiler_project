#include "AST.h"

// Function to print the tree
std::unique_ptr<ASTNode> printTree(std::unique_ptr<ASTNode> head) {
    if (!head) return nullptr; // Base case for recursion

    // Print the content of the node based on its type
    if (auto numberNode = dynamic_cast<NumberExprAST*>(head.get())) {
        std::cout << "Number: " << numberNode->print() << std::endl;
    } else if (auto variableNode = dynamic_cast<VariableExprAST*>(head.get())) {
        std::cout << "Variable: " << variableNode->print() << std::endl;
    } else if (auto binaryNode = dynamic_cast<BinaryExprAST*>(head.get())) {
        std::cout << "Binary Operation: " << binaryNode->print() << std::endl;
        printTree(std::move(binaryNode->getLHS())); // Recursively print LHS
        printTree(std::move(binaryNode->getRHS())); // Recursively print RHS
    } else if (auto conditionNode = dynamic_cast<ConditionAST*>(head.get())) {
        std::cout << "Condition: " << conditionNode->print() << std::endl;
        printTree(std::move(conditionNode->getCond())); // Recursively print condition
        for (auto& node : conditionNode->getBlock()) {
            printTree(std::move(node)); // Recursively print block nodes
        }
    } else if (auto loopNode = dynamic_cast<LoopAST*>(head.get())) {
        std::cout << "Loop: " << loopNode->print() << std::endl;
        printTree(std::move(loopNode->getCond())); // Recursively print loop condition
        for (auto& node : loopNode->getBlock()) {
            printTree(std::move(node)); // Recursively print block nodes
        }
    } else if (auto callNode = dynamic_cast<CallExprAST*>(head.get())) {
        std::cout << "Function Call: " << callNode->getCallee() << std::endl;
        for (auto& arg : callNode->getArgs()) {
            printTree(std::move(arg)); // Recursively print arguments
        }
    } else if (auto prototypeNode = dynamic_cast<PrototypeAST*>(head.get())) {
        std::cout << "Prototype: " << prototypeNode->getName() << std::endl;
    } else if (auto functionNode = dynamic_cast<FunctionAST*>(head.get())) {
        std::cout << "Function" << std::endl;
        for (auto& node : functionNode->getBody()) {
            printTree(std::move(node)); // Recursively print body nodes
        }
    } else if (auto programNode = dynamic_cast<ProgramAST*>(head.get())) {
        std::cout << "Program" << std::endl;
        for (auto& func : programNode->getFunctions()) {
            printTree(std::move(func)); // Recursively print functions
        }
    }

    return std::move(head); // Return the unique pointer
}
