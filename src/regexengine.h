#pragma once
#include <vector>
#include <string>
#include <unordered_map>
#include <map>
#include <functional>

enum class TokenType {
    NONE,
    LITERAL,
    DIGIT,
    WORD,
    NOT_WORD,
    NOT_DIGIT,
    SPACE,
    NOT_SPACE,
    WILD_CARD,
    CHAR_GROUP,
    NEGATED_GROUP,
    ANCHOR_START,
    ANCHOR_END,
    ALTERNATION
};

struct Token {
    TokenType type;
    std::string value;
    std::function<bool(const std::string&, size_t&, const Token&)> func;
    int min_rep;
    int max_rep;
    std::vector<Token> alternatives = {};
};

class RegexEngine {
public:
    RegexEngine();
    std::vector<Token> parser(const std::string& pattern);
    std::vector<std::pair<size_t, size_t>> match(std::vector<Token>& input_tokens, std::string& word);
    std::pair<int, Token> deduce_type(const std::string& pattern, std::vector<Token>& temp_tokens);
    std::pair<int, std::vector<Token>> handle_inside_brackets(int id, const std::string& pattern, size_t end_bracket_pos);

     //temp helper for debugging
    std::map<int, std::vector<Token>>& get_saved();

private:
    //member variables
    std::vector<Token> tokens;
    std::map<std::pair<size_t, size_t>, std::pair<size_t, bool>> memo;
    std::unordered_map<int, std::function<bool(const std::string&, size_t&, const Token&)>> func_register;
    std::map<int, std::vector<Token>> saved_brackets;

    void register_functions();
    Token handle_escape(char c);
    void handle_quantifiers(std::vector<Token>& tokens, char c);
    std::pair<size_t, bool> try_match(std::string& word, size_t i, size_t p);

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
    bool wildcard(const std::string& input, size_t& input_pos, const Token& token);
    bool alternation(const std::string& input, size_t& input_pos, const Token& token);
};