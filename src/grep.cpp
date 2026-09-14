#include "grep.h"
#include "regexengine.h"
#include <iostream>
#include <vector>
#include <string>

int Grep::handle_grep(const std::vector<std::string>& tokens) {
    /*
    std::string flag = (tokens[0][0] == '-') ? tokens[0] : "";
    if (flag == "-E") {
        handle_regex
    } else {
        handle_literal
    }
    */
    std::string pattern = tokens[0];
    std::string word = tokens[1];
    std::vector<Token> parsed_tokens = RegexEngine::parser(pattern);
    bool found;
    found = RegexEngine::match(parsed_tokens, word);
    for (const auto& t : regex_res) {
        std::cout << (int)t.type << " : " << t.val << std::endl;
    }
}