#include "grep.h"
#include <iostream>
#include <string>

int Grep::handle_grep(const std::vector<std::string>& tokens) {
    std::string pattern = tokens[0];
    std::string word = tokens[1];
    if (pattern[0] == '[' && pattern[pattern.size()-1] == ']') {
        std::string chars = pattern.substr(1, pattern.size() - 2);  
        for (char c : word) {
            if (chars.find(c) != std::string::npos) { 
                return 0;  
            }
        }
        return 1;  
    }
    if (pattern == "\\w") {
        for (char c : word) {
            if (std::isalnum(c) || c == '_') {
                return 0; 
            }
        }
    } else if (pattern == "\\d") {
        for (size_t i = 0; i < word.size(); i++) {
            if (std::isdigit(word[i])) {
                return 0;  
            }
        }
    } 
    return 1;
}