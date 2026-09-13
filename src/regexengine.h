#pragma once

#include <string>

enum class TokenType {
    LITERAL,
    DIGIT,          // \d
    WORD,           // \w
    CHAR_GROUP,     // [abc]
    NEGATED_GROUP,  // [^abc]
    
    //quantifiers 
    STAR,           // *
    PLUS,           // +
    QUESTION,       // ?
    
    //advanced 
    ANCHOR_START,   // ^
    ANCHOR_END,     // $
    ALTERNATION,    // |
    GROUP,          // (...)
};

struct Token {
    TokenType type;
    std::string val;
    int min_occ = 1;
    int max_occ = 1;
};

class RegexEngine {
public:
    std::vector<Token> parser(const std::string& pattern);
};