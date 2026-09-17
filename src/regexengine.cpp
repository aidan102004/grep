#include "regexengine.h"
#include <string>
#include <cctype>
#include <iostream>

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
    func_register[static_cast<int>(TokenType::WILD_CARD)] = [this](const std::string& input, size_t& pos, const Token& token) {
        return wildcard(input, pos, token);
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
                tokens.push_back({TokenType::LITERAL, "[", func_register[static_cast<int>(TokenType::LITERAL)], 1, 1}); //if we cannot find it then push the [ as a literal
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
        } else if (c == '+' || c == '*' || c == '?') { //handle quantifiers
            if (!tokens.empty()) 
                handle_quantifiers(tokens, c); 
            else 
                tokens.push_back({TokenType::LITERAL, std::string(1, c), func_register[static_cast<int>(TokenType::LITERAL)], 1, 1});
        } else if (c == '{') {
            size_t end = pattern.find('}', i + 1);
            if (end == std::string::npos) {
                tokens.push_back({TokenType::LITERAL, "{", func_register[static_cast<int>(TokenType::LITERAL)], 1, 1}); //if we cannot find it then push the { as a literal
            } else {
                if (end-i == 2) {
                    tokens.back().min_rep = pattern[end - 1] - '0';
                    tokens.back().max_rep = pattern[end - 1]- '0';
                } else if (end-i == 3) {
                    tokens.back().min_rep = (int)pattern[i+1]- '0';
                    tokens.back().max_rep = INT_MAX;
                } else {
                    tokens.back().min_rep = (int)pattern[i+1]- '0';
                    tokens.back().max_rep = (int)pattern[i+3]- '0';
                }
                i = end;
            }
        } else if( c == '.') {
            tokens.push_back({TokenType::WILD_CARD, std::string(1, c), func_register[static_cast<int>(TokenType::WILD_CARD)], 1, 1});
        } else {
            tokens.push_back({TokenType::LITERAL, std::string(1, c), func_register[static_cast<int>(TokenType::LITERAL)], 1, 1});
        }    
    }
    return tokens;
}


void RegexEngine::handle_quantifiers(std::vector<Token>& tokens, char c) {
    if (c == '+') {
        tokens.back().min_rep = 1;
        tokens.back().max_rep = INT_MAX;
    } else if (c == '*') {
        tokens.back().min_rep = 0;
        tokens.back().max_rep = INT_MAX;
    } else if (c == '?') {
        tokens.back().min_rep = 0;
        tokens.back().max_rep = 1;
    }
}

std::vector<std::pair<size_t, size_t>> RegexEngine::match(std::vector<Token>& input_tokens, std::string& word) {
    tokens = input_tokens;
    std::cout << tokens.size() << std::endl;
    std::vector<std::pair<size_t, size_t>> res;
    for (size_t s = 0; s < word.size(); s++) {
        auto [end_index, matched] = try_match(word, s, 0);
        if (matched) {
            res.push_back({s, end_index});
            s = end_index;
        }
    }
    return res; //recursively match word with tokens at respective positions
}

std::pair<size_t, bool> RegexEngine::try_match(std::string& word, size_t i, size_t p) {
    //base case if we have exceeded the length of tokens meaning we have completed them all
    if (p >= tokens.size()) {
        return {i, true}; //return true by default now because we search through the entire string
    }
    
    //memoisation, check if input and pattern pos pair exists in memo
    auto key = std::make_pair(i, p);
    auto it = memo.find(key);
    if (it != memo.end()) {
        return it->second; //in the case it does return precomputed result
    }
    Token& token = tokens[p];
    std::pair<size_t, bool> result = {i, false};

    if (token.type == TokenType::ANCHOR_START || token.type == TokenType::ANCHOR_END) {
        if (token.func(word, i, token)) {
            result = try_match(word, i, p + 1);
        }
    } else {
        std::vector<size_t> match_positions; //store match positions for backtracking
        match_positions.push_back(i); //add the first position 
        
        size_t temp_pos = i; //store pos
        int matches = 0; //increment matches
        
        //greedily match as many times as possible
        while (matches < token.max_rep && temp_pos < word.size() && token.func(word, temp_pos, token)) {
            matches++; //increment matches
            match_positions.push_back(temp_pos);  //record position after this match
        }
        
        //backtrack by trying to match from longest match down to minimum
        int max_reps = (int)match_positions.size() - 1;
        for (int reps = max_reps; reps >= token.min_rep; reps--) {
            if (reps < 0) break;
            size_t pos_holder = match_positions[reps];
            auto [end_index, matched] = try_match(word, pos_holder, p + 1);
            if (matched) { //once we find a match on the following token in the pattern begin recursion again
                result.second = true;
                result.first = end_index;
                break;
            }
        }
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
    return pos == input.size() || pos == '\n';
}
bool RegexEngine::wildcard(const std::string& input, size_t& pos, const Token& token) {
    return input[pos] != '\n';
}