#include "BuildBasic.h"

llvm::Module* Basic::GenerateLLVM()
{
    auto programAST = dynamic_cast<ProgramAST *>(root.get());
    if (!programAST) {
        throw std::runtime_error("Invalid AST: root is not a ProgramAST");
    }

    for (const auto &i : programAST->getFunctions())
    {
        auto functionAST = dynamic_cast<FunctionAST *>(i.get());
        if (!functionAST) {
            throw std::runtime_error("Invalid AST: function is not a FunctionAST");
        }
        GenerateFunction(functionAST);
    }
    return module.get();
}

llvm::Function *Basic::GenerateFunction(FunctionAST *node)
{
    auto proto = dynamic_cast<PrototypeAST *>(node->getProto().get());
    if (!proto) {
        throw std::runtime_error("Invalid AST: node prototype is not a PrototypeAST");
    }
    
    std::string functionName = proto->getName();
    auto returnType = llvm::Type::getDoubleTy(context);
    std::vector<llvm::Type *> Types(proto->getArgs().size(), llvm::Type::getDoubleTy(context));
    auto parmTypes = llvm::ArrayRef<llvm::Type *>(Types);

    // Create the LLVM function
    auto funcType = llvm::FunctionType::get(returnType, parmTypes, false);
    function = llvm::Function::Create(funcType, llvm::Function::ExternalLinkage, functionName, module.get());

    ifCount = 0;
    whileCount = 0;
    entryCount = 0;
    size_t pc = 0;
    std::vector<std::unique_ptr<ASTNode>>* functionBody = &node->getBody();
    llvm::BasicBlock *Block = llvm::BasicBlock::Create(context, "entry" + std::to_string(entryCount), function);

    // Generate the basic block instructions
    ControllerBasicBlock(Block,functionBody, pc); // Updated function name

    return function;
}

void Basic::ControllerBasicBlock(llvm::BasicBlock *curBlock, std::vector<std::unique_ptr<ASTNode>>* block , size_t pc) // Updated function name
{
    llvm::BasicBlock *mergeBlock = curBlock;
    bool used = true;
    builder.SetInsertPoint(mergeBlock);
    while (pc < block->size())
    {
        if (block->at(pc)->type() == "if"&& pc+1 < block->size() && block->at(pc + 1)->type() == "else")
        {
            entryCount++;
            if (used == false)
            {
                builder.SetInsertPoint(mergeBlock);
            }
            auto conditionAST = dynamic_cast<ConditionAST*>(block->at(pc).get());
            if (!conditionAST) {
                throw std::runtime_error("Invalid AST: block is not a ConditionAST");
            }
            auto conditionValue = GenerateExpression(conditionAST->getCond().get(),false);

            auto thenBlock = llvm::BasicBlock::Create(context, "then" + std::to_string(ifCount), function);
            auto elseBlock = llvm::BasicBlock::Create(context, "else" + std::to_string(ifCount), function);
            mergeBlock = llvm::BasicBlock::Create(context, "entry" + std::to_string(entryCount), function);
            used = false;
            builder.CreateCondBr(conditionValue, thenBlock, elseBlock);

            ifCount++;

            // builder.SetInsertPoint(thenBlock);
            ControllerBasicBlock(thenBlock, &conditionAST->getBlock() ,0); // Updated function name
            builder.CreateBr(mergeBlock);

            conditionAST = dynamic_cast<ConditionAST*>(block->at(pc+1).get());
            if (!conditionAST) {
                throw std::runtime_error("Invalid AST: block is not a ConditionAST");
            }

            // builder.SetInsertPoint(elseBlock);
            ControllerBasicBlock(elseBlock, &conditionAST->getBlock(), 0); // Updated function name
            builder.CreateBr(mergeBlock);

            pc += 2;
        }
        else if (block->at(pc)->type() == "if")
        {
            entryCount++;
            if (used == false)
            {
                builder.SetInsertPoint(mergeBlock);
            }
            
            auto conditionAST = dynamic_cast<ConditionAST*>(block->at(pc).get());
            if (!conditionAST) {
                throw std::runtime_error("Invalid AST: block is not a ConditionAST");
            }

            auto conditionValue = GenerateExpression(conditionAST->getCond().get(),false);

            auto thenBlock = llvm::BasicBlock::Create(context, "then" + std::to_string(ifCount), function);
            mergeBlock = llvm::BasicBlock::Create(context, "entry" + std::to_string(entryCount), function);
            used = false;
            builder.CreateCondBr(conditionValue, thenBlock, mergeBlock);

            // builder.SetInsertPoint(thenBlock);
            ControllerBasicBlock(thenBlock, &conditionAST->getBlock(), 0);
            builder.CreateBr(mergeBlock);

            ifCount++;
            pc++;
        }
        else if (block->at(pc)->type() == "While")
        {
            entryCount++;
            if (used == false)
            {
                builder.SetInsertPoint(mergeBlock);
            }
            auto loopAST = dynamic_cast<LoopAST*>(block->at(pc).get());
            if (!loopAST) {
                throw std::runtime_error("Invalid AST: block is not a ConditionAST");
            }

            auto conditionValue = GenerateExpression(loopAST->getCond().get(),false);

            auto loopBlock = llvm::BasicBlock::Create(context, "while" + std::to_string(whileCount), function);
            mergeBlock = llvm::BasicBlock::Create(context, "entry" + std::to_string(entryCount), function);
            used = false;
            builder.CreateCondBr(conditionValue, loopBlock, mergeBlock);

            // builder.SetInsertPoint(loopBlock);
            ControllerBasicBlock(loopBlock, &loopAST->getBlock(), 0);
            builder.CreateBr(mergeBlock);
            pc++;
        }
        else
        {
            used = true;
            GenerateExpression(block->at(pc).get(), false);
            pc++;
        }
    }
    if (used == false)
    {
        builder.SetInsertPoint(mergeBlock);
    }
}

llvm::Value *Basic::GenerateExpression(ASTNode *node, bool left)
{
    if (node->type() == "Operator")
    {
        
        auto binaryExpr = dynamic_cast<BinaryExprAST*>(node);
        llvm::Value * leftValue;

        std::cout<<binaryExpr->getOp()<<std::endl;

        auto rightValue = GenerateExpression(binaryExpr->getRHS().get(), false);
        if (binaryExpr->getOp() == "=")
        {
            std::cout<<"i am called"<<std::endl;
            leftValue = GenerateExpression(binaryExpr->getLHS().get(), true);
        }else{
            leftValue = GenerateExpression(binaryExpr->getLHS().get(), false);
        }
        
        if (binaryExpr->getOp() == "+")
        {
            return builder.CreateFAdd(leftValue, rightValue, "add");
        }
        else if (binaryExpr->getOp() == "-")
        {
            return builder.CreateFSub(leftValue, rightValue, "sub");
        }
        else if (binaryExpr->getOp() == "*")
        {
            return builder.CreateFMul(leftValue, rightValue, "mul");
        }
        else if (binaryExpr->getOp() == "/")
        {
            return builder.CreateFDiv(leftValue, rightValue, "div");
        }
        else if (binaryExpr->getOp() == "<")
        {
            return builder.CreateFCmpULT(leftValue, rightValue, "lt");
        }
        else if (binaryExpr->getOp() == "<=")
        {
            return builder.CreateFCmpULE(leftValue, rightValue, "le");
        }
        else if (binaryExpr->getOp() == ">=")
        {
            return builder.CreateFCmpUGE(leftValue, rightValue, "ge");
        }
        else if (binaryExpr->getOp() == "==")
        {
            return builder.CreateFCmpOEQ(leftValue, rightValue, "eq");
        }
        else if (binaryExpr->getOp() == "<>")
        {
            return builder.CreateFCmpONE(leftValue, rightValue, "ne");
        }
        else if (binaryExpr->getOp() == "=")
        {
            
            return builder.CreateStore(leftValue, rightValue);
        }
    }
    else if (node->type() == "Variable")
    {   
        auto varExpr = dynamic_cast<VariableExprAST*>(node);
        auto name= GetName(varExpr->getName(),left);
        return name;
    }
    else if (node->type() == "Number")
    {
        auto numExpr = dynamic_cast<NumberExprAST*>(node);
        return llvm::ConstantFP::get(llvm::Type::getDoubleTy(context), numExpr->getValue());
    }
}

llvm::AllocaInst* Basic::GetName(std::string name, bool left){
    std::cout<<name<<"--"<<left<<std::endl;
    if(std::find(variables.begin(), variables.end(), name) == variables.end()){
        table[name] = builder.CreateAlloca(llvm::Type::getDoubleTy(context), nullptr,name);
        variables.push_back(name);
        count[name] = 1;
        return table[name];
    } else {
        if (!left) {
            return table[name];
        } else {
            count[name]++;
            table[name] =builder.CreateAlloca(llvm::Type::getDoubleTy(context), nullptr,name + std::to_string(count[name]));
            return table[name];
        }
    }
}

void Basic::print(){
    module->print(llvm::outs(),nullptr);
}
