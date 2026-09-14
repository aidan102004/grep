#include "regexengine.h"
#include <string>

RegexEngine::RegexEngine(){
    register_functions();
}

void RegexEngine::register_functions() {
    func_register[TokenType::LITERAL] = [this](const std::string& input, const std::string& pattern, size_t pos) {
        return literal(input, pattern, pos);
    };
    func_register[TokenType::DIGIT] = [this](const std::string& input, const std::string& pattern, size_t pos) {
        return digit(input, pattern, pos);
    };
    func_register[TokenType::WORD] = [this](const std::string& input, const std::string& pattern, size_t pos) {
        return word(input, pattern, pos);
    };
    func_register[TokenType::CHAR_GROUP] = [this](const std::string& input, const std::string& pattern, size_t pos) {
        return char_group(input, pattern, pos);
    };
    func_register[TokenType::NEGATED_GROUP] = [this](const std::string& input, const std::string& pattern, size_t pos) {
        return negated_group(input, pattern, pos);
    };
    func_register[TokenType::NOT_WORD] = [this](const std::string& input, const std::string& pattern, size_t pos) {
        return notword(input, pattern, pos);
    };
    func_register[TokenType::NOT_DIGIT] = [this](const std::string& input, const std::string& pattern, size_t pos) {
        return notdig(input, pattern, pos);
    };
    func_register[TokenType::SPACE] = [this](const std::string& input, const std::string& pattern, size_t pos) {
        return space(input, pattern, pos);
    };
    func_register[TokenType::NOT_SPACE] = [this](const std::string& input, const std::string& pattern, size_t pos) {
        return notspace(input, pattern, pos);
    };
    func_register[TokenType::ANCHOR_START] = [this](const std::string& input, const std::string& pattern, size_t pos) {
        return start(input, pattern, pos);
    };
    func_register[TokenType::ANCHOR_END] = [this](const std::string& input, const std::string& pattern, size_t pos) {
        return end(input, pattern, pos);
    };
}

std::vector<Token> RegexEngine::parser(const std::string& pattern) {
    std::vector<Token> tokens;
    for (size_t i = 0; i < pattern.size(); i++) {
        char c = pattern[i];
        if (c == '\\') {
            handle_escape(tokens, pattern[i + 1]);
            i++;
        } else if (c == '[') {
            size_t end = pattern.find(']', i + 1);
            
            if (end == std::string::npos) {
                tokens.push_back({TokenType::LITERAL, "[", func_register[TokenType::LITERAL], 1, 1});
            } else {
                std::string chars = pattern.substr(i + 1, end - i - 1);
                if (!chars.empty() && chars[0] == '^') {
                    tokens.push_back({TokenType::NEGATED_GROUP, chars.substr(1), func_register[TokenType::NEGATED_GROUP], 1, 1});
                } else {
                    tokens.push_back({TokenType::CHAR_GROUP, chars, func_register[TokenType::CHAR_GROUP], 1, 1});
                }         
            i = end;
            }
        } else if (c == '^') {
            tokens.push_back({TokenType::ANCHOR_START, std::string(1, c), func_register[TokenType::ANCHOR_START], 1, 1});
        } else if (c == '$') {
            tokens.push_back({TokenType::ANCHOR_END, std::string(1, c), func_register[TokenType::ANCHOR_END], 1, 1});
        } else {
            tokens.push_back({TokenType::LITERAL, std::string(1, c), func_register[TokenType::LITERAL], 1, 1});
        }    
    }
    return tokens;

}

bool RegexEngine::match(std::vector<Token>& input_tokens, std::string& word) {
    tokens = input_tokens;
    return try_match(word, 0, 0);
}
bool RegexEngine::try_match(std::string& word, size_t i, size_t p) {
    if (p > tokens.size()) {
        return true;
    }
    auto key = std::make_pair(i, p);
    auto it = memo.find(key);
    if (it != memo.end()) {
        return it->second;
    }

    bool result = false;
    if (i > word.size() && tokens[p].func(word, i)) {
        result = try_match(word, i + 1, p + 1);
    }
    memo[key] = result;
    return result;
}

void RegexEngine::handle_escape(std::vector<Token>& tokens, char c) {
    static const std::unordered_map<char, TokenType> escape_map = { //refactor to class scope later
        {'d', TokenType::DIGIT},
        {'w', TokenType::WORD},
        {'W', TokenType::NOT_WORD},
        {'D', TokenType::NOT_DIGIT},
        {'s', TokenType::SPACE},
        {'S', TokenType::NOT_SPACE},
    };

    auto it = escape_map.find(c);
    if (it != escape_map.end()) {
        tokens.push_back({it->second, std::string(1, c), func_register[it->second], 1, 1});
    } else {
        tokens.push_back({TokenType::LITERAL, std::string(1, c), func_register[TokenType::LITERAL], 1, 1});
    }
}


bool RegexEngine::literal(const std::string& input, const std::string& pattern, size_t& pos) {
    return false;
}

bool RegexEngine::digit(const std::string& input, const std::string& pattern, size_t& pos) {
    return false;
}

bool RegexEngine::word(const std::string& input, const std::string& pattern, size_t& pos) {
    return false;
}

bool RegexEngine::notword(const std::string& input, const std::string& pattern, size_t& pos) {
    return false;
}

bool RegexEngine::notdig(const std::string& input, const std::string& pattern, size_t& pos) {
    return false;
}

bool RegexEngine::space(const std::string& input, const std::string& pattern, size_t& pos) {
    return false;
}

bool RegexEngine::notspace(const std::string& input, const std::string& pattern, size_t& pos) {
    return false;
}

bool RegexEngine::char_group(const std::string& input, const std::string& pattern, size_t& pos) {
    return false;
}

bool RegexEngine::negated_group(const std::string& input, const std::string& pattern, size_t& pos) {
    return false;
}

bool RegexEngine::start(const std::string& input, const std::string& pattern, size_t& pos) {
    return false;
}

bool RegexEngine::end(const std::string& input, const std::string& pattern, size_t& pos) {
    return false;
}
