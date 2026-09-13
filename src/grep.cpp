#include "grep.h"
#include "regexengine.h"
#include <iostream>
#include <vector>
#include <string>

int Grep::handle_grep(const std::vector<std::string>& tokens) {
    std::string pattern = tokens[0];
    std::string word = tokens[1];
    std::vector<Token> regex_res = regex_engine.parser(pattern);
    for (const auto& t : regex_res) {
        std::cout << (int)t.type << " : " << t.val << std::endl;
    }
}