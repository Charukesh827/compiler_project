#include <iostream>
#include <fstream>
#include <sstream>
#include "lexer/Lexer.h"
#include "parser/Parser.h"

std::string readFile(const std::string& filename) {
    std::ifstream file(filename);
    std::stringstream buffer;
    buffer << file.rdbuf();
    return buffer.str();
}

int main() {
    std::string source = readFile("/home/hajun/aaaaa/compiler_project/examples/code1.txt");
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

    return 0;
}
