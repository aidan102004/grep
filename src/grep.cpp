#include "grep.h"
#include "regexengine.h"
#include <iostream>
#include <vector>
#include <string>
#include <cstring>
#include <unistd.h>
#include <filesystem>
#include <fstream>
#include <sstream>
void load_rs(std::string file_name);

const std::string RED = "\033[31m";
const std::string GREEN = "\033[32m";
const std::string YELLOW = "\033[33m";
const std::string BLUE = "\033[34m";
const std::string BOLD = "\033[1m";
const std::string BOLD_RED = "\033[1;31m";
const std::string BOLD_GREEN = "\033[1;32m";
const std::string RESET = "\033[0m";

Grep::Grep() {
    register_functions();
}

/*registers function pointers to map so we can call functions for all flag types*/
void Grep::register_functions() {
    flag_map["E"] = [this](std::string& input) {
        preferences.use_extended_regex = true;
    };
    flag_map["m"] = [this](std::string& input) {
        if (!input.empty()) {
            try {
                preferences.num_matches = std::stoi(input);
            } catch (const std::invalid_argument&) {
                std::cerr << "Error: '" << input << "' is not a valid number" << std::endl;
                preferences.num_matches = INT_MAX;
            } catch (const std::out_of_range&) {
                std::cerr << "Error: '" << input << "' is out of range" << std::endl;
                preferences.num_matches = INT_MAX;
            }
        }
    };
    flag_map["o"] = [this](std::string& input) {
        preferences.print_matches_only = true;
    };
    flag_map["r"] = [this](std::string& input) {
        preferences.recursive_search = true;
    };
    flag_map["color"] = [this](std::string& input) {
        preferences.option = input;
    };
}

void Grep::handle_grep(const std::vector<std::string>& tokens) {
    
    auto [pattern, contents] = parse(tokens); //returns both the pattern and the word
    std::vector<FileLine> text;
    //filename and file contents
    for (const auto& file : contents) {
        //if any of the tokens following the pattern are files we run the search on those files, not the literal text
        if (file_exists(file)) {
            //if we are recursively searching the directory
            if (std::filesystem::is_directory(file) && preferences.recursive_search) {
                std::vector<FileLine> temp = load_rs(file);
                text.insert(text.end(), temp.begin(), temp.end());
                fn_count++;
                continue;
            }
            std::vector<std::string> lines = read_file(file);
            fn_count++; //how we check if there are multiple files
            for (const auto& line : lines) {
                text.push_back({file, line});
            }
        }
    }
    if (text.empty()) {
        std::string joined;
        for (const auto& w : contents) {
            joined += w + " ";
        }
        text.push_back({"", joined});
    }
    if (preferences.use_extended_regex) { 
        handle_regex(pattern, text); //hanlde regex
    } else {
        //handle_literal(text); //handle normal string search
    }
    preferences.reset(); //reset modifications done by flags everytime
}

void Grep::handle_regex(const std::string& pattern, const std::vector<FileLine>& file_lines) {
    std::vector<Token> parsed_tokens = engine.parser(pattern);
    bool multi_file = fn_count > 1; //multifile check
    int matches_count = 0;
    for (const auto& fl : file_lines) {
        //return a vector of pairs of indices representing the start and end positions of a match within a string
        std::vector<std::pair<size_t, size_t>> matches = engine.match(parsed_tokens, fl.content);
        if (matches.empty()) continue; //this confirms we have matches
        matches_count++;
        //set prefix depending if its multifile
        std::string fn = std::filesystem::path(fl.file_name).filename().string();
        std::string prefix = (multi_file && !fl.file_name.empty()) ? fn + ":" : ""; 
        if (preferences.recursive_search && !fl.file_name.empty()) prefix = fl.file_name + ":"; //sets prefix to dir if we recursively searched

        if (!preferences.print_matches_only)
            print_matches(fl.content, matches, should_colorise(preferences.option.c_str()), preferences.num_matches, prefix);
        else
            print_only_matches(fl.content, matches, should_colorise(preferences.option.c_str()), preferences.num_matches, prefix);
    }
    if (matches_count == 0) {
        std::cout << "grep: no matches found" << std::endl;
    }
}

std::vector<FileLine> Grep::load_rs(std::string file) {
    std::vector<FileLine> files;
    //recursivly search directories and read files into line vector 
    for (std::filesystem::recursive_directory_iterator it(file), end; it != end; ++it) {
        if (!std::filesystem::is_directory(it->path())) {
            std::ifstream file(it->path());
            if (!file.is_open()) {
                continue; //dont work with files that cant be opened
            }
            std::vector<std::string> lines = read_file(it->path().string()); //read file into lines
            for (const auto l : lines) {
                files.push_back({it->path().parent_path().string(), l}); //add parent path and contents to files
            }
        }
    }
    return files;
}

void Grep::handle_literal(const std::vector<std::string>& word) {
    //todo, implement literal search
}

std::pair<std::string, std::vector<std::string>> Grep::parse(const std::vector<std::string>& tokens) {
    if (tokens.size() < 2) return {"", {}};  //saftey check
    
    size_t index = 1; //and increment to keep track of our position in tokens
    std::string input = "";
    std::string pattern;
    std::string final;
    bool num_required = false; //switched if -m
    std::vector<std::string> flags;
    
    //search through token by token so we handle cases with combined flags and seperate flags
    while (index < tokens.size() && tokens[index][0] == '-') {
        if (tokens[index].size() > 1 && tokens[index][1] == '-') { //handle --color case
            size_t pos = tokens[index].find('=');
            if (pos != std::string::npos) {
                input = tokens[index].substr(pos + 1); 
                flags.push_back(tokens[index].substr(2, pos - 2)); 
            }
        } else {
            //add all characters to flag vector to be handled
            for (const auto& c : tokens[index].substr(1)) {
                if (c == 'm') num_required = true;
                flags.push_back(std::string(1, c));
            }
        }
        index++;
    }
    if (num_required) {
        if (index < tokens.size()) {   
            input = tokens[index];
            index++; //increment one more as we consumed it                   
        }
    }
    //for each flag run its respective func ptr to set preferences
    for (const auto& f : flags) {
        auto it = flag_map.find(f);
        if (it != flag_map.end()) {
            it->second(input); //func runs here
        }
    }
    if (index < tokens.size()) {  
        pattern = tokens[index]; //set pattern
        index++;
    }
    std::vector<std::string> sub(tokens.begin() + index, tokens.end());
    return {pattern, sub};
}

/*read file contents line by line*/
std::vector<std::string> Grep::read_file(const std::string& path) {
    std::ifstream file(path);
    if (!file.is_open()) {
        std::cerr << "grep: " << path << ": No such file or directory" << std::endl;
        return {};
    }
    std::vector<std::string> lines;
    std::string line;
    while (std::getline(file, line)) {
        lines.push_back(line);
    }
    return lines;
}


/*checks whether we should apply colour to the output*/
bool Grep::should_colorise(const char* option) {
    if (strcmp(option, "always") == 0) return true;
    if (strcmp(option, "never") == 0) return false;
    return isatty(STDOUT_FILENO); 
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
        case TokenType::WILD_CARD: return "WILDCARD";
        case TokenType::ALTERNATION: return "ALTERNATION";
        default: return "UNKNOWN";
    }
}

/*print matches within string*/
void Grep::print_matches(const std::string& word, const std::vector<std::pair<size_t, size_t>>& indicies, bool coloured, int count, const std::string& prefix) {
    std::string final = prefix;
    size_t i = 0;
    for (const auto& [start, end] : indicies) {
        if (count == 0) break;
        final += word.substr(i, start - i);
        std::string modified_text = (coloured) ? colorise(word.substr(start, end - start), BOLD_RED) : word.substr(start, end - start);
        final += modified_text;
        i = end;
        count--;
    }
    final += word.substr(i);
    std::cout << final << std::endl;
}

/*print only matches*/
void Grep::print_only_matches(const std::string& word, const std::vector<std::pair<size_t, size_t>>& indicies, bool coloured, int count, const std::string& prefix) {
    for (const auto& [start, end] : indicies) {
        if (count == 0) break;
        std::string modified_text = (coloured) ? colorise(word.substr(start, end - start), BOLD_RED) : word.substr(start, end - start);
        std::cout << modified_text << std::endl;
        count--;
    }
}

/*helper for printing tokens so i can debug*/
void Grep::print_helper(std::vector<Token>& tokens) {
    for (const auto& t : tokens) {
        std::cout << tokenTypeToString(t.type) << " : " << t.value << " min-rep: " << t.min_rep << " | max-rep: " << t.max_rep << std::endl;
    }
}

std::string Grep::colorise(const std::string& text, const std::string& color) {
    return color + text + RESET;
}
