#ifndef TOKEN_H
#define TOKEN_H

#include <string>

enum class TokenType {
    EOF_TOKEN,      // End of file
    IDENTIFIER,     // Variable names
    NUMBER,         // Numeric literals
    OPERATOR,       // Operators (+, -, *, /, etc.)
    PUNCTUATION,    // Symbols like (, ), ;
    KEYWORD,        // Language keywords (e.g., def, extern)
    INVALID,        // Unrecognized token
    EOL_TOKEN,      // End of line ( ; )
    
};

struct Token {
    TokenType type;
    std::string value;
    int line;
    int column;

    Token(TokenType t, const std::string &val, int ln, int col)
        : type(t), value(val), line(ln), column(col) {}
};

#endif // TOKEN_H
