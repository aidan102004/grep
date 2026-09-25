#pragma once
#include <string>
#include <iostream>
#include <functional>
#include "regexengine.h"
#include <sys/stat.h>
#include <unistd.h>

using FlagFunction = std::function<void(std::string&)>;


const std::string RESET = "\033[0m";

struct FileLine {
    std::string file_name;
    std::string content;
    int line_num;
};


struct Preferences {
    bool use_extended_regex = false;
    bool print_matches_only = false;
    int num_matches = INT_MAX;
    bool recursive_search = false;
    bool print_count = false;
    bool display_line_nums = false;
    bool case_insensitive = false;
    std::string option = "auto";
    std::string COLOR = "\033[1;31m";

    /* we need to reset preferences after each command */
    void reset() {
        use_extended_regex = false;
        print_matches_only = false;
        print_count = false;
        display_line_nums = false;
        case_insensitive = false;
        num_matches = INT_MAX;
    }
};

class Grep {
public:
    Grep();
    void handle_grep(const std::vector<std::string>& command);
private:
    //preferences
    Preferences preferences = {false, false, INT_MAX, false, false, false, false, "auto"};

    //func ptrs for flags
    std::unordered_map<std::string, FlagFunction> flag_map;
    int fn_count = 0;
    //regex engine
    RegexEngine engine;
    void register_functions();
    std::vector<FileLine> load_rs(std::string file_name);
    std::pair<std::string, std::vector<std::string>> parse(const std::vector<std::string>& tokens);
    std::vector<std::string> read_file(const std::string& path);
    void handle_pattern(const std::string& pattern, const std::vector<FileLine>& words);
    void print_helper(std::vector<Token>& tokens);
    void print_matches(const std::string& word, const std::vector<std::pair<size_t, size_t>>& indicies, bool coloured, int count, const std::string& prefix);
    void print_only_matches(const std::string& word, const std::vector<std::pair<size_t, size_t>>& indicies, bool coloured, int count, const std::string& prefix);
    std::string colorise(const std::string& text, const std::string& color);
    bool should_colorise(const char* option);
    std::string get_escape_code(const std::string& input);
    std::vector<std::pair<size_t, size_t>> boyer_moore(const std::string& text, const std::string& pattern);
    void display_help_popup();
    std::string get_help_text();
    /*checks if this string is actually also a file*/
    inline bool file_exists (const std::string& name) {
        struct stat buffer;   
        return (stat (name.c_str(), &buffer) == 0); 
    }
};