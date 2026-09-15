#pragma once
#include <vector>
#include <string>
#include <unordered_map>
#include <map>
#include <functional>

enum class TokenType {
    LITERAL,
    DIGIT,
    WORD,
    NOT_WORD,
    NOT_DIGIT,
    SPACE,
    NOT_SPACE,
    CHAR_GROUP,
    NEGATED_GROUP,
    ANCHOR_START,
    ANCHOR_END
};

struct Token {
    TokenType type;
    std::string value;
    std::function<bool(const std::string&, size_t&, const Token&)> func;
    int min_rep;
    int max_rep;
};

class RegexEngine {
public:
    RegexEngine();
    std::vector<Token> parser(const std::string& pattern);
    bool match(std::vector<Token>& input_tokens, std::string& word);

private:
    std::vector<Token> tokens;
    std::map<std::pair<size_t, size_t>, bool> memo;
    std::unordered_map<int, std::function<bool(const std::string&, size_t&, const Token&)>> func_register;

    void register_functions();
    void handle_escape(std::vector<Token>& tokens, char c);
    bool try_match(std::string& word, size_t i, size_t p);

    //these functions handle each tokens match check
    bool literal(const std::string& input, size_t& input_pos, const Token& token);
    bool digit(const std::string& input, size_t& input_pos, const Token& token);
    bool word(const std::string& input, size_t& input_pos, const Token& token);
    bool notdig(const std::string& input, size_t& input_pos, const Token& token);
    bool notword(const std::string& input, size_t& input_pos, const Token& token);
    bool space(const std::string& input, size_t& input_pos, const Token& token);
    bool notspace(const std::string& input, size_t& input_pos, const Token& token);
    bool char_group(const std::string& input, size_t& input_pos, const Token& token);
    bool negated_group(const std::string& input, size_t& input_pos, const Token& token);
    bool start(const std::string& input, size_t& input_pos, const Token& token);
    bool end(const std::string& input, size_t& input_pos, const Token& token);
};