#include "grep.h"
#include "regexengine.h"
#include <iostream>
#include <vector>
#include <string>

const std::string RED = "\033[31m";
const std::string GREEN = "\033[32m";
const std::string YELLOW = "\033[33m";
const std::string BLUE = "\033[34m";
const std::string BOLD = "\033[1m";
const std::string BOLD_RED = "\033[1;31m";
const std::string BOLD_GREEN = "\033[1;32m";
const std::string RESET = "\033[0m";

int Grep::handle_grep(const std::vector<std::string>& tokens) {
    std::string pattern = tokens[0];
    std::string word = tokens[1];
    RegexEngine engine;
    std::vector<Token> parsed_tokens = engine.parser(pattern); //parse
    print_helper(parsed_tokens);
    std::vector<std::pair<size_t, size_t>> matches_pair = engine.match(parsed_tokens, word); //match
    print_matches(word, matches_pair);
    return matches_pair.size();
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

void Grep::print_matches(const std::string& word, const std::vector<std::pair<size_t, size_t>>& indicies) {
    std::string final;
    size_t i = 0;
    for (const auto& [start, end] : indicies) {
        final += word.substr(i, start - i);
        final += colorise(word.substr(start, end - start), BOLD_RED);
        i = end;
    }
    final += word.substr(i);
    std::cout << final << std::endl;
}

/*helper for printing tokens so i can debug*/
void Grep::print_helper(const std::vector<Token>& tokens) {
    for (const auto& t : tokens) {
        std::cout << tokenTypeToString(t.type) << " : " << t.value << " min-rep: " << t.min_rep << " | max-rep: " << t.max_rep << std::endl;
    }
}

std::string Grep::colorise(const std::string& text, const std::string& color) {
    return color + text + RESET;
}
