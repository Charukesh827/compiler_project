#include <iostream>
#include <fstream>
#include <sstream>
#include "lexer/Lexer.h"
#include "parser/Parser.h"
#include "LLVM/BuildBasic.h"

std::string readFile(const std::string& filename) {
    std::ifstream file(filename);
    std::stringstream buffer;
    buffer << file.rdbuf();
    return buffer.str();
}

int main() {
    std::string source = readFile("/home/hajun/aaaaa/compiler_project/examples/code2.txt");
    // Lexer
    Lexer lexer(source);
    auto tokens = lexer.tokenize();

    for (const auto &token : tokens) {
        std::cout << "Token: " << token.value << " (" << static_cast<int>(token.type) << ") - ["<<token.line<<" , "<<token.column<<"]\n";
    }

    // Parser
    Parser parser(tokens);
    auto ast = parser.parse();

    std::cout << "Parsing completed!\n";

    Basic code = Basic(std::move(ast));

    code.GenerateLLVM();

    code.print();
    
    return 0;
}
