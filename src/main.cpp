#include <iostream>
#include <string>
#include <sstream>
#include <vector>
#include "grep.h"

Grep grep;

void parser(std::vector<std::string>& tokens, const std::string& command);

int main() {
    std::cout << std::unitbuf; 
    std::cerr << std::unitbuf;
    while (true) {
        std::cout << "$ ";
        std::string command;
        std::getline(std::cin, command);
        if (command.empty()) continue;
        std::vector<std::string> tokens;
        parser(tokens, command);
        std::string output = (grep.handle_grep(tokens) == 0) ? "true" : "false"; //start dispatch process
        std::cout << output << std::endl;
    }
}

void parser(std::vector<std::string>& tokens, const std::string& command) {
    std::string cur = "";
    for (size_t i = 0; i < command.size(); i++) {
        char c = command[i];
        if (c == ' ') {
            tokens.push_back(cur);
            cur.clear();
        } else {
            cur += c;
        }
    }
    if (!cur.empty()) {
        tokens.push_back(cur);
        cur.clear();
    }
}