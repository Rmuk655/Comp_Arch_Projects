/**
 * @file Parser.cpp
 * @brief Implementation of the Parser class utility functions for string manipulation.
 *
 * This file contains helper methods for:
 *   - Trimming whitespace from strings.
 *   - Removing comments and extraneous whitespace from lines.
 *   - Splitting strings by a specified delimiter.
 *
 * Functions:
 *   - std::string Parser::trim(const std::string& str) const
 *       Removes leading and trailing whitespace (spaces and tabs) from the input string.
 *
 *   - std::string Parser::cleanLine(const std::string& line) const
 *       Removes comments (anything after '#') and trims whitespace from the input line.
 *
 *   - std::vector<std::string> Parser::split(const std::string& str, char delimiter) const
 *       Splits the input string by the specified delimiter, trimming whitespace from each token.
 */
#include "Parser.h"
#include <sstream>


// Helper to trim whitespace and to remove empty lines
std::string Parser::trim(const std::string& str) const {
    size_t first = str.find_first_not_of(" \t");
    size_t last = str.find_last_not_of(" \t");
    return (first == std::string::npos) ? "" : str.substr(first, last - first + 1);
}

//Remove comments and trim whitespace
std::string Parser::cleanLine(const std::string& line) const {
    // Remove comment
    size_t commentPos = line.find('#');
    std::string noComment = (commentPos != std::string::npos) ? line.substr(0, commentPos) : line;

    // Trim the line
    std::string trimmed = trim(noComment);

    return trimmed; // May be empty
}

// Helper to split a string by delimiters
std::vector<std::string> Parser::split(const std::string& str, char delimiter) const {
    std::vector<std::string> tokens;
    std::stringstream ss(str);
    std::string item;
    while (std::getline(ss, item, delimiter)) {
        tokens.push_back(trim(item));
    }
    return tokens;
}
