#include "grep.h"
#include <iostream>
#include <string>

int Grep::handle_grep(const std::vector<std::string>& tokens) {
    std::string word = tokens[0];
    const std::string target_letter = "\\d";
    for (size_t i = 0; i < word.size(); i++) {
        if (word[i] == target_letter[0]) {
            return 0;
        }
    }
    return 1;
}