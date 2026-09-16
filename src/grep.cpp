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
    RegexEngine engine;
    std::vector<Token> parsed_tokens = engine.parser(pattern); //parse
    print_helper(parsed_tokens);
    bool found;
    found = engine.match(parsed_tokens, word); //match
    std::string status = (found == true) ? "found" : "no found";
    std::cout << status << std::endl;
    return (int)found;
}

/*helper for printing tokens so i can debug*/
std::string tokenTypeToString(TokenType type) {
    switch (type) {
        case TokenType::LITERAL: return "LITERAL";
        case TokenType::DIGIT: return "DIGIT";
        case TokenType::WORD: return "WORD";
        case TokenType::NOT_WORD: return "NOT_WORD";
        case TokenType::NOT_DIGIT: return "NOT_DIGIT";
        case TokenType::SPACE: return "SPACE";
        case TokenType::NOT_SPACE: return "NOT_SPACE";
        case TokenType::CHAR_GROUP: return "CHAR_GROUP";
        case TokenType::NEGATED_GROUP: return "NEGATED_GROUP";
        case TokenType::ANCHOR_START: return "ANCHOR_START";
        case TokenType::ANCHOR_END: return "ANCHOR_END";
        default: return "UNKNOWN";
    }
}

/*helper for printing tokens so i can debug*/
void Grep::print_helper(const std::vector<Token>& tokens) {
    for (const auto& t : tokens) {
        std::cout << tokenTypeToString(t.type) << " : " << t.value << " min-rep: " << t.min_rep << " | max-rep: " << t.max_rep << std::endl;
    }
}
