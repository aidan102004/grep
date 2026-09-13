#pragma once

#include <string>

using token_func = std::function<bool(const std::string& input, const std::string& pattern, size_t& input_pos)>;

enum class TokenType {
    LITERAL,
    DIGIT,          // \d
    NOT_DIGIT,
    WORD,           // \w
    NOT_WORD,
    CHAR_GROUP,     // [abc]
    NEGATED_GROUP,  // [^abc]
    SPACE,
    NOT_SPACE,
    
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
    token_func func;
    int min_occ = 1;
    int max_occ = 1;

    Token(TokenType type, std::string val, token_func func, int min_occ, int max_occ) 
        : type(type), val(val), func(func), min_occ(min_occ), max_occ(max_occ) {}
};

class RegexEngine {
    std::vector<Token> tokens;
    std::unordered_map<TokenType, token_func> func_register;
public:
    RegexEngine();
    void register_functions();
    std::vector<Token> parser(const std::string& pattern);
    void handle_escape(std::vector<Token>& tokens, char c);
    bool literal(const std::string& input, const std::string& pattern, size_t& input_pos);
    bool digit(const std::string& input, const std::string& pattern, size_t& input_pos);
    bool word(const std::string& input, const std::string& pattern, size_t& input_pos);
    bool notdig(const std::string& input, const std::string& pattern, size_t& input_pos);
    bool notword(const std::string& input, const std::string& pattern, size_t& input_pos);
    bool space(const std::string& input, const std::string& pattern, size_t& input_pos);
    bool notspace(const std::string& input, const std::string& pattern, size_t& input_pos);
    bool char_group(const std::string& input, const std::string& pattern, size_t& input_pos);
    bool negated_group(const std::string& input, const std::string& pattern, size_t& input_pos);
    bool star(const std::string& input, const std::string& pattern, size_t& input_pos);
    bool plus(const std::string& input, const std::string& pattern, size_t& input_pos);
    bool question(const std::string& input, const std::string& pattern, size_t& input_pos);
    bool start(const std::string& input, const std::string& pattern, size_t& input_pos);
    bool end(const std::string& input, const std::string& pattern, size_t& input_pos);
};