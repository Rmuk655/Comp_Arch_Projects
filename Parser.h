#ifndef PARSER_H
#define PARSER_H

#include <string>
#include <vector>
#include <unordered_map>


class Parser {
public:
    Parser()= default;

    // Helper to remove comments, trim whitespace and to remove empty lines
    std::string cleanLine(const std::string& line) const ;
        
    // Helper to trim whitespace
    std::string trim(const std::string& str) const;

    // Helper to split a string by delimiters
    std::vector<std::string> split(const std::string& str, char delimiter) const;

};

#endif // ASSEMBLER_H
