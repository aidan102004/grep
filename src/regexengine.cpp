#include "regexengine.h"
#include <string>
#include <cctype>

RegexEngine::RegexEngine(){
    register_functions();
}

/*register functions to map*/
void RegexEngine::register_functions() {
    func_register[static_cast<int>(TokenType::LITERAL)] = [this](const std::string& input, size_t& pos, const Token& token) {
        return literal(input, pos, token);
    };
    func_register[static_cast<int>(TokenType::DIGIT)] = [this](const std::string& input, size_t& pos, const Token& token) {
        return digit(input, pos, token);
    };
    func_register[static_cast<int>(TokenType::WORD)] = [this](const std::string& input, size_t& pos, const Token& token) {
        return word(input, pos, token);
    };
    func_register[static_cast<int>(TokenType::CHAR_GROUP)] = [this](const std::string& input, size_t& pos, const Token& token) {
        return char_group(input, pos, token);
    };
    func_register[static_cast<int>(TokenType::NEGATED_GROUP)] = [this](const std::string& input, size_t& pos, const Token& token) {
        return negated_group(input, pos, token);
    };
    func_register[static_cast<int>(TokenType::NOT_WORD)] = [this](const std::string& input, size_t& pos, const Token& token) {
        return notword(input, pos, token);
    };
    func_register[static_cast<int>(TokenType::NOT_DIGIT)] = [this](const std::string& input, size_t& pos, const Token& token) {
        return notdig(input, pos, token);
    };
    func_register[static_cast<int>(TokenType::SPACE)] = [this](const std::string& input, size_t& pos, const Token& token) {
        return space(input, pos, token);
    };
    func_register[static_cast<int>(TokenType::NOT_SPACE)] = [this](const std::string& input, size_t& pos, const Token& token) {
        return notspace(input, pos, token);
    };
    func_register[static_cast<int>(TokenType::ANCHOR_START)] = [this](const std::string& input, size_t& pos, const Token& token) {
        return start(input, pos, token);
    };
    func_register[static_cast<int>(TokenType::ANCHOR_END)] = [this](const std::string& input, size_t& pos, const Token& token) {
        return end(input, pos, token);
    };
}


/*parse pattern text into tokens*/
std::vector<Token> RegexEngine::parser(const std::string& pattern) {
    std::vector<Token> tokens;
    for (size_t i = 0; i < pattern.size(); i++) {
        char c = pattern[i];
        if (c == '\\') {
            handle_escape(tokens, pattern[i + 1]); //handles all potential escape patterns
            i++;
        } else if (c == '[') {
            size_t end = pattern.find(']', i + 1); //finds pos of ] which we use to set i
            
            if (end == std::string::npos) {
                tokens.push_back({TokenType::LITERAL, "[", func_register[static_cast<int>(TokenType::LITERAL)], 1, 1}); //if we cannot find it then push the ] as a literal
            } else {
                std::string chars = pattern.substr(i + 1, end - i - 1);
                if (!chars.empty() && chars[0] == '^') { //check for negation
                    tokens.push_back({TokenType::NEGATED_GROUP, chars.substr(1), func_register[static_cast<int>(TokenType::NEGATED_GROUP)], 1, 1});
                } else {
                    tokens.push_back({TokenType::CHAR_GROUP, chars, func_register[static_cast<int>(TokenType::CHAR_GROUP)], 1, 1});
                }         
            i = end; //set increment
            }
        } else if (c == '^') {
            tokens.push_back({TokenType::ANCHOR_START, std::string(1, c), func_register[static_cast<int>(TokenType::ANCHOR_START)], 1, 1});
        } else if (c == '$') {
            tokens.push_back({TokenType::ANCHOR_END, std::string(1, c), func_register[static_cast<int>(TokenType::ANCHOR_END)], 1, 1});
        } else {
            tokens.push_back({TokenType::LITERAL, std::string(1, c), func_register[static_cast<int>(TokenType::LITERAL)], 1, 1});
        }    
    }
    return tokens;
}

bool RegexEngine::match(std::vector<Token>& input_tokens, std::string& word) {
    tokens = input_tokens;
    return try_match(word, 0, 0); //recursively match word with tokens at respective positions
}

bool RegexEngine::try_match(std::string& word, size_t i, size_t p) {
    //base case if we have exceeded the length of tokens meaning we have completed them all
    if (p >= tokens.size()) {
        return i == word.size(); //return true if input pos is at end of word
    }
    
    //memoisation, check if input and pattern pos pair exists in memo
    auto key = std::make_pair(i, p);
    auto it = memo.find(key);
    if (it != memo.end()) {
        return it->second; //in the case it does return precomputed result
    }

    bool result = false;
    if (i < word.size() && tokens[p].func(word, i, tokens[p])) { //if we arent at the end of the word and the func ptr rtrns true
        result = try_match(word, i, p + 1);  //we hanlde i increment in function not here
    }
    
    memo[key] = result; //set key now that we have computed
    return result; //return result
}

void RegexEngine::handle_escape(std::vector<Token>& tokens, char c) {
    static const std::unordered_map<char, TokenType> escape_map = {
        {'d', TokenType::DIGIT},
        {'w', TokenType::WORD},
        {'W', TokenType::NOT_WORD},
        {'D', TokenType::NOT_DIGIT},
        {'s', TokenType::SPACE},
        {'S', TokenType::NOT_SPACE},
    };

    auto it = escape_map.find(c);
    if (it != escape_map.end()) {
        tokens.push_back({it->second, std::string(1, c), func_register[static_cast<int>(it->second)], 1, 1});
    } else {
        tokens.push_back({TokenType::LITERAL, std::string(1, c), func_register[static_cast<int>(TokenType::LITERAL)], 1, 1});
    }
}

bool RegexEngine::literal(const std::string& input, size_t& pos, const Token& token) {
    if (pos < input.size() && input[pos] == token.value[0]) {
        pos++;
        return true;
    }
    return false;
}

bool RegexEngine::digit(const std::string& input, size_t& pos, const Token& token) {
    if (pos < input.size() && std::isdigit(static_cast<unsigned char>(input[pos]))) {
        pos++;
        return true;
    }
    return false;
}

bool RegexEngine::word(const std::string& input, size_t& pos, const Token& token) {
    if (pos < input.size() && (std::isalnum(static_cast<unsigned char>(input[pos])) || input[pos] == '_')) {
        pos++;
        return true;
    }
    return false;
}

bool RegexEngine::notword(const std::string& input, size_t& pos, const Token& token) {
    if (pos < input.size() && !std::isalnum(static_cast<unsigned char>(input[pos])) && input[pos] != '_') {
        pos++;
        return true;
    }
    return false;
}

bool RegexEngine::notdig(const std::string& input, size_t& pos, const Token& token) {
    if (pos < input.size() && !std::isdigit(static_cast<unsigned char>(input[pos]))) {
        pos++;
        return true;
    }
    return false;
}

bool RegexEngine::space(const std::string& input, size_t& pos, const Token& token) {
    if (pos < input.size() && std::isspace(static_cast<unsigned char>(input[pos]))) {
        pos++;
        return true;
    }
    return false;
}

bool RegexEngine::notspace(const std::string& input, size_t& pos, const Token& token) {
    if (pos < input.size() && !std::isspace(static_cast<unsigned char>(input[pos]))) {
        pos++;
        return true;
    }
    return false;
}

bool RegexEngine::char_group(const std::string& input, size_t& pos, const Token& token) {
    if (pos < input.size() && token.value.find(input[pos]) != std::string::npos) {
        pos++;
        return true;
    }
    return false;
}

bool RegexEngine::negated_group(const std::string& input, size_t& pos, const Token& token) {
    if (pos < input.size() && token.value.find(input[pos]) == std::string::npos) {
        pos++;
        return true;
    }
    return false;
}

bool RegexEngine::start(const std::string& input, size_t& pos, const Token& token) {
    return pos == 0;
}

bool RegexEngine::end(const std::string& input, size_t& pos, const Token& token) {
    return pos == input.size();
}