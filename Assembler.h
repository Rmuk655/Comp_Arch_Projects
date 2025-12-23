
/**
 * @class Assembler
 * @brief Assembles assembly language source code into machine code instructions.
 *
 * The Assembler class provides functionality to parse, assemble, and encode assembly language
 * instructions into their corresponding machine code representations. It manages label resolution,
 * instruction parsing, and provides mappings between source lines and program counters for debugging
 * and simulation purposes.
 *
 * Key Features:
 * - Parses assembly source lines and encodes them into 32-bit instructions.
 * - Handles label collection and resolution for branch and jump instructions.
 * - Maintains a mapping between source lines and program counters.
 * - Provides access to parsed instructions and label sets.
 * - Supports error logging and instruction printing for debugging.
 *
 * Dependencies:
 * - InstructionSet: Defines the supported instruction formats and encodings.
 * - InstructionInstance: Represents a parsed instruction instance.
 * - Parser: Tokenizes and parses assembly lines.
 * - LabelSet: Manages label definitions and lookups.
 *
 * Usage:
 * Instantiate with input and output streams, then call assemble() with source lines.
 */
#ifndef ASSEMBLER_H
#define ASSEMBLER_H

#include <string>
#include <vector>
#include <unordered_map>
#include <iostream>
#include "InstructionSet.h"
#include "InstructionInstance.h"
#include "Parser.h"
#include "LabelSet.h"

class Assembler
{

public:
    Assembler(std::istream &in = std::cin, std::ostream &out = std::cout, ISAMode mode = ISAMode::RV32I)
        : in(in), out(out), instructionSet(mode), parser() {}

    // Parses assembly lines and returns a vector of encoded instructions
    std::vector<uint32_t> assemble(const std::vector<std::string> &lines);
    // Getter for labelSet
    const LabelSet &getLabelSet() const;

    // Getter for instructions
    const std::vector<InstructionInstance> &getInstructions() const;

    const std::unordered_map<int, int> &getSourceLineToPC() const;
    void reset();
    void printInstructions() const;
    void printBinInstructions() const;

    int getCurrentSourceLine(int pc) const;
    void printFormattedAssembly() const;
    
    // Get line number in full formatted assembly (including labels) by PC
    int getFormattedAssemblyLineNumberByPC(uint32_t pc) const;
    
    // Get all formatted lines as a vector (useful for indexing)
    std::vector<std::string> getAllFormattedLines() const;
protected:
    std::istream &in;
    std::ostream &out;

private:

    std::vector<uint32_t> machineCodes;
    const InstructionSet instructionSet;
    const Parser parser;
    LabelSet labelSet;
    std::vector<InstructionInstance> instructions;
    // Convert register name like "x5" to integer 5
    int parseRegister(const std::string &token) const;

    // Convert immediate string to integer when labels can be present
    int32_t parseImmediate(const std::string &token, int currentLine) const;

    // parse memory operand like "4(x5)" into immediate and register
    bool parseMemoryOperand(const std::string &operand, int &imm, int &rs1, int pc) const;

    // Collects labels in LabelSet and returns instruction lines only (with label definitions removed)
    std::vector<std::string> collectLabels(const std::vector<std::string> &lines);

    // For simulator: source line → program counter mapping
    std::unordered_map<int, int> sourceLineToPC; 

    void logError(const std::string& message, int pc = -1) const;
    // Helper method to build the sorted formatted lines
    std::vector<std::pair<uint32_t, std::string>> buildFormattedLines() const;
   


};

#endif // ASSEMBLER_H
