#include "BuildBasic.h"

llvm::Module* Basic::GenerateLLVM()
{
    std::cout<<"Generating LLVM"<<std::endl;
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

    std::cout<<"Generating Function "<<functionName<<std::endl;

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
    variables.empty();
    table.empty();
    count.empty();

    std::vector<std::unique_ptr<ASTNode>>* functionBody = &node->getBody();
    llvm::BasicBlock *Block = llvm::BasicBlock::Create(context, "entry" + std::to_string(entryCount), function);

    // Generate the basic block instructions
    ControllerBasicBlock(Block,functionBody, pc); // Updated function name

    return function;
}

void Basic::ControllerBasicBlock(llvm::BasicBlock *curBlock, std::vector<std::unique_ptr<ASTNode>>* block , size_t pc) // Updated function name
{
    std::cout<<"generating Basic Block "<<std::string(curBlock->getName())<<std::endl;
    llvm::BasicBlock *mergeBlock = curBlock;
    llvm::BasicBlock *conditionblock;
    bool used = true;
    builder.SetInsertPoint(mergeBlock);
    std::cout<<"Block Size: "<<block->size()<<std::endl;
    while (pc < block->size())
    {
        std::cout<<"pc: "<<pc<<std::endl;
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
            //finding the variables that are used in if
            auto beforeIF = count;

            // builder.SetInsertPoint(thenBlock);
            ControllerBasicBlock(thenBlock, &conditionAST->getBlock() ,0); // Updated function name
            builder.CreateBr(mergeBlock);

            auto afterIF = count;
            auto variableIF = findMismatches(beforeIF,afterIF);
            std::cout<<"if keys: "<<variableIF.size()<<std::endl;

            conditionAST = dynamic_cast<ConditionAST*>(block->at(pc+1).get());
            if (!conditionAST) {
                throw std::runtime_error("Invalid AST: block is not a ConditionAST");
            }

            //finding the variables that are used in else
            auto beforeELSE = count;

            // builder.SetInsertPoint(elseBlock);
            ControllerBasicBlock(elseBlock, &conditionAST->getBlock(), 0); // Updated function name
            builder.CreateBr(mergeBlock);

            auto afterELSE = count;
            auto variableELSE = findMismatches(beforeELSE,afterELSE);
            std::cout<<"else keys: "<<variableELSE.size()<<std::endl;

            auto commonVariable = findCommonKeys(variableIF,variableELSE);
            std::cout<<"common keys: "<<commonVariable.size()<<std::endl;

            if (commonVariable.size() > 0){

                builder.SetInsertPoint(mergeBlock);
                for (auto &i : commonVariable)
                {
                    llvm::Value* ifValue = builder.CreateFPExt(variableIF[i], llvm::Type::getDoubleTy(context), "ifValue"); // Cast if necessary
                    llvm::Value* elseValue = builder.CreateFPExt(variableELSE[i], llvm::Type::getDoubleTy(context), "elseValue"); // Cast if necessary                    
                    
                    std::string name = std::string(GetName(i,true)->getName());
                    auto var = builder.CreatePHI(llvm::Type::getDoubleTy(context), 2,name);
                    var->addIncoming(ifValue,thenBlock);
                    var->addIncoming(elseValue,elseBlock);
                }
                
            }
            used=true;
            pc += 2;
        }
        else if (block->at(pc)->type() == "if")
        {
            std::cout<<"I am doing it"<<std::endl;
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
            auto conditionblock = llvm::BasicBlock::Create(context, "cond"+std::to_string(whileCount),function);
            builder.SetInsertPoint(conditionblock);
            auto conditionValue = GenerateExpression(loopAST->getCond().get(),false);

            auto loopBlock = llvm::BasicBlock::Create(context, "while" + std::to_string(whileCount), function);
            mergeBlock = llvm::BasicBlock::Create(context, "entry" + std::to_string(entryCount), function);
            used = false;
            builder.CreateCondBr(conditionValue, loopBlock, mergeBlock);

            
            ControllerBasicBlock(loopBlock, &loopAST->getBlock(), 0);
            builder.CreateBr(conditionblock);
            pc++;
        }
        else
        {
            if(used==false){
                builder.SetInsertPoint(mergeBlock);
                used = true;
            }
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
        }else if (binaryExpr->getOp() == ">")
        {
            return builder.CreateFCmpUGT(leftValue, rightValue, "gt");
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

std::map<std::string, llvm::AllocaInst *> Basic::findMismatches(const std::map<std::string, int>& map1, const std::map<std::string, int>& map2) {
    std::map<std::string, llvm::AllocaInst *> mismatches;

    // Check for keys in map1
    for (const auto& pair : map1) {
        const std::string& key = pair.first;
        int count1 = pair.second;

        // Check if the key exists in map2
        auto it = map2.find(key);
        if (it != map2.end()) {
            int count2 = it->second;
            // If counts are different, add to mismatches
            if (count1 != count2) {
                mismatches[key]=table[key];
            }
        } else {
            // Key is not in map2
            mismatches[key] = table[key];
        }
    }

    // Check for keys that are only in map2
    for (const auto& pair : map2) {
        const std::string& key = pair.first;

        // If key is not found in map1, add it to mismatches
        if (map1.find(key) == map1.end()) {
            mismatches[key] = table[key];
        }
    }

    return mismatches;
}


std::vector<std::string> Basic::findCommonKeys(const std::map<std::string, llvm::AllocaInst*>& map1, const std::map<std::string, llvm::AllocaInst*>& map2) {
    std::vector<std::string> commonKeys;

    // Iterate through the first map
    for (const auto& pair : map1) {
        const std::string& key = pair.first;

        // Check if the key exists in the second map
        if (map2.find(key) != map2.end()) {
            // If it exists in both maps, add to commonKeys
            commonKeys.push_back(key);
        }
    }

    return std::move(commonKeys);
}