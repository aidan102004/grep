#pragma once
#include <string>
#include <iostream>
#include <functional>
#include "regexengine.h"

using FlagFunction = std::function<void(std::string&)>;

struct Preferences {
    bool use_extended_regex = false;
    bool print_matches_only = false;
    int num_matches = INT_MAX;
    std::string option = "auto";

    /* we need to reset preferences after each command */
    void reset() {
        use_extended_regex = false;
        print_matches_only = false;
        num_matches = INT_MAX;
    }
};

class Grep {
public:
    Grep();
    void handle_grep(const std::vector<std::string>& command);
private:
    //preferences
    Preferences preferences = {false, false, INT_MAX, "auto"};

    //func ptrs for flags
    std::unordered_map<std::string, FlagFunction> flag_map;
    //regex engine
    RegexEngine engine;
    void register_functions();
    std::pair<std::string, std::string> parse(const std::vector<std::string>& tokens);
    void handle_regex(const std::string& pattern, const std::string& word);
    void handle_literal(const std::string& word);
    void print_helper(std::vector<Token>& tokens);
    void print_matches(const std::string& word, const std::vector<std::pair<size_t, size_t>>& indicies, bool coloured, int count);
    void print_only_matches(const std::string& word, const std::vector<std::pair<size_t, size_t>>& indicies, bool coloured, int count);
    std::string colorise(const std::string& text, const std::string& color);
    bool should_colorise(const char* option);
};