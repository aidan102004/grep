#include "regexengine.h"
#include <string>
#include <cctype>
#include <iostream>
#include <stack>

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
    func_register[static_cast<int>(TokenType::ALTERNATION)] = [this](const std::string& input, size_t& pos, const Token& token) {
        return alternation(input, pos, token);
    };
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


/*parse pattern text into tokens*/
std::vector<Token> RegexEngine::parser(const std::string& pattern) {
    std::vector<Token> tokens;
    for (size_t i = 0; i < pattern.size(); i++) {
        if (pattern[i] == '(') {
            size_t end = pattern.find(')', i + 1);
            if (end == std::string::npos) {
                tokens.push_back({TokenType::LITERAL, std::string(1, pattern[i]), func_register[static_cast<int>(TokenType::LITERAL)], 1, 1});
            } else {
                auto [index, temp] = handle_inside_brackets(saved_brackets.size() + 1, pattern.substr(i + 1), end);
                if (!temp.empty()) {
                    tokens.insert(tokens.end(), temp.begin(), temp.end());
                    i += index;
                }
            }
        } else {
            auto [index, token] = deduce_type(pattern.substr(i), tokens);
            if (token.type != TokenType::NONE) tokens.push_back(token);
            i += index;
        }
    }
    return tokens;
}

std::pair<int, std::vector<Token>> RegexEngine::handle_inside_brackets(int id, const std::string& pattern, size_t end_bracket_pos) {
    std::vector<Token> temp_tokens;
    int i = 0;
    char cur_char;
    cur_char = pattern[i];
    while (cur_char != ')') {
        if (pattern[i] == '(') {
            size_t next = pattern.find(')', i + 1);
            if (next == std::string::npos) {
                temp_tokens.push_back({TokenType::LITERAL, "(", func_register[static_cast<int>(TokenType::LITERAL)], 1, 1});
                i++;
            } else {
                auto [index, temp_vector] = handle_inside_brackets(id + 1, pattern.substr(i + 1), end_bracket_pos);
                temp_tokens.insert(temp_tokens.end(), temp_vector.begin(), temp_vector.end());
                saved_brackets[id].insert(saved_brackets[id].end(), temp_vector.begin(), temp_vector.end());
                int increment = (temp_vector[0].type == TokenType::ALTERNATION) ? 0 : 1;
                i += index + increment;
            }
        } else {
            std::pair<int, Token> token = deduce_type(pattern.substr(i), temp_tokens);
            i += token.first + 1;
            if (token.second.type == TokenType::NONE) {
                saved_brackets[id][saved_brackets[id].size() - 1].max_rep = temp_tokens.back().max_rep;
                saved_brackets[id][saved_brackets[id].size() - 1].min_rep = temp_tokens.back().min_rep;
            } else {
                if (i < pattern.size() && pattern[i] == '|' && temp_tokens.size() == 0) {
                    token.second.alternatives.push_back(token.second);
                    token.second.type = TokenType::ALTERNATION;
                    token.second.func = func_register[static_cast<int>(TokenType::ALTERNATION)];
                    std::vector<std::string> alts;
                    std::string current;
                    for (size_t p = i + 1; p < end_bracket_pos; p++) {
                        if (pattern[p] == '|') {
                            if (!current.empty()) {
                                alts.push_back(current);
                            }
                            current = "";
                        } else {
                            current += pattern[p];
                        }
                    }
                    if (!current.empty()) alts.push_back(current);
                    for (const auto& alt : alts) {
                        std::pair<int, Token> t = deduce_type(alt, temp_tokens);
                        token.second.alternatives.push_back(t.second);
                    }
                    saved_brackets[id].push_back(token.second);
                    temp_tokens.push_back(token.second);
                    i = end_bracket_pos - 1;
                    break;
                } else {
                    temp_tokens.push_back(token.second);
                    saved_brackets[id].push_back(token.second);
                }
        }
            }
            
        cur_char = pattern[i];
    }
    i++;
    return {i, temp_tokens};
}

std::pair<int, Token> RegexEngine::deduce_type(const std::string& pattern, std::vector<Token>& temp_tokens) {
        char c = pattern[0];
        int i = 0;
        if (c == '\\') {
            return {i + 1 , handle_escape(pattern[i+1])}; 
        } else if (c == '[') {
            size_t end = pattern.find(']', i + 1); //finds pos of ] which we use to set i
            
            if (end == std::string::npos) {
                return {i, {TokenType::LITERAL, "[", func_register[static_cast<int>(TokenType::LITERAL)], 1, 1}}; //if we cannot find it then push the [ as a literal
            } else {
                std::string chars = pattern.substr(i + 1, end - i - 1);
                if (!chars.empty() && chars[0] == '^') { //check for negation
                    return {end, {TokenType::NEGATED_GROUP, chars.substr(1), func_register[static_cast<int>(TokenType::NEGATED_GROUP)], 1, 1}};
                } else {
                    return {end, {TokenType::CHAR_GROUP, chars, func_register[static_cast<int>(TokenType::CHAR_GROUP)], 1, 1}};
                }         
            }
        } else if (c == '^') {
            return {i, {TokenType::ANCHOR_START, std::string(1, c), func_register[static_cast<int>(TokenType::ANCHOR_START)], 1, 1}};
        } else if (c == '$') {
            return {i, {TokenType::ANCHOR_END, std::string(1, c), func_register[static_cast<int>(TokenType::ANCHOR_END)], 1, 1}};
        } else if (c == '+' || c == '*' || c == '?') { //handle quantifiers
            if (!temp_tokens.empty()) {
                handle_quantifiers(temp_tokens, c); 
                return {i, {TokenType::NONE, std::string(1, c), func_register[static_cast<int>(TokenType::LITERAL)], 1, 1}};
            } else  {
                return {i, {TokenType::LITERAL, std::string(1, c), func_register[static_cast<int>(TokenType::LITERAL)], 1, 1}};
            }
        } else if (c == '{') { //handle exactly, at least and between quantifiers
            size_t end = pattern.find('}', i + 1); //check for closing bracket from the following pos
            if (end == std::string::npos) {
                return {i,{TokenType::LITERAL, "{", func_register[static_cast<int>(TokenType::LITERAL)], 1, 1}}; //if we cannot find it then push the { as a literal
            } else {
                if (end-i == 2) { // {n} case
                    temp_tokens.back().min_rep = pattern[end - 1] - '0'; //cast char to int
                    temp_tokens.back().max_rep = pattern[end - 1]- '0';
                } else if (end-i == 3) { //{n,} case
                    temp_tokens.back().min_rep = (int)pattern[i+1]- '0';
                    temp_tokens.back().max_rep = INT_MAX;
                } else { // {n,m} case
                    temp_tokens.back().min_rep = (int)pattern[i+1]- '0';
                    temp_tokens.back().max_rep = (int)pattern[i+3]- '0';
                }
                i = end; //increment i to end
                return {i, {TokenType::NONE, std::string(1, c), func_register[static_cast<int>(TokenType::LITERAL)], 1, 1}};
            }
        } else if( c == '.') {
            return {i, {TokenType::WILD_CARD, std::string(1, c), func_register[static_cast<int>(TokenType::WILD_CARD)], 1, 1}};
        } else {
            return {i, {TokenType::LITERAL, std::string(1, c), func_register[static_cast<int>(TokenType::LITERAL)], 1, 1}};
        }    
    }


std::vector<std::pair<size_t, size_t>> RegexEngine::match(std::vector<Token>& input_tokens, std::string& word) {
    tokens = input_tokens; 
    std::vector<std::pair<size_t, size_t>> res; //setup container to store index pairs
    for (size_t s = 0; s < word.size(); s++) { //increment through the whole piece of text starting searching at every letter
        auto [end_index, matched] = try_match(word, s, 0); //in the case we successfuly find a match we insert it and increment s
        if (matched) { //this ensures we only pushback actual matches
            res.push_back({s, end_index});
            s = end_index; 
        }
    }
    return res; 
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
    std::pair<size_t, bool> result = {i, false}; //setup return variable

    if (token.type == TokenType::ANCHOR_START || token.type == TokenType::ANCHOR_END) { //we handle start and end anchors seperatly so we dont need to check the word length
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
            if (reps < 0) break; //this checks in the case there is no matches and it wont go negative and give us a seg fault
            size_t pos_holder = match_positions[reps]; //reference holder for incrementing the pos
            auto [end_index, matched] = try_match(word, pos_holder, p + 1);
            //if we find a match set values
            if (matched) { 
                result.second = true;
                result.first = end_index;
                break;
            }
        }
    }
    
    memo[key] = result; //set key now that we have computed
    return result; //return result
}

Token RegexEngine::handle_escape(char c) {
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
        return {it->second, std::string(1, c), func_register[static_cast<int>(it->second)], 1, 1};
    } else {
        return {TokenType::LITERAL, std::string(1, c), func_register[static_cast<int>(TokenType::LITERAL)], 1, 1};
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
    return pos == input.size() || (pos < input.size() && input[pos] == '\n');
}

bool RegexEngine::wildcard(const std::string& input, size_t& pos, const Token& token) {
    if (pos < input.size() && input[pos] != '\n') {
        pos++; 
        return true;
    }
    return false;
}

bool RegexEngine::alternation(const std::string& input, size_t& pos, const Token& token) {
    size_t original_pos = pos;  
    for (const auto& t : token.alternatives) {
        pos = original_pos;  
        if (t.func(input, pos, t)) {
            return true;
        }
    }
    return false;
}

std::map<int, std::vector<Token>>& RegexEngine::get_saved() {
    return saved_brackets;
}