#pragma once
#include <string>
#include <iostream>
#include "regexengine.h"

class Grep {
public:
    int handle_grep(const std::vector<std::string>& command);
    void print_matches(const std::string& word, const std::vector<std::pair<size_t, size_t>>& indicies);
    void print_helper(const std::vector<Token>& tokens);
    std::string colorise(const std::string& text, const std::string& color);
};