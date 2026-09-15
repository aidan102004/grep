#pragma once
#include <string>
#include <iostream>
#include "regexengine.h"

class Grep {
public:
    int handle_grep(const std::vector<std::string>& command);
    void print_helper(const std::vector<Token>& tokens);
};