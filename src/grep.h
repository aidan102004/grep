#pragma once
#include <string>
#include <iostream>
#include "regexengine.h"

class Grep {
private:
    RegexEngine regex_engine;
public:
    int handle_grep(const std::vector<std::string>& command);
};