/*
 * Assembler.cpp
 *
 * This file implements the Assembler class, which is responsible for parsing, assembling,
 * and managing RISC-V assembly code. The Assembler provides functionality to:
 *   - Parse assembly lines, collect and manage labels, and map source lines to program counters.
 *   - Parse registers (both numeric and ABI names) and immediate values, including label references.
 *   - Parse memory operands in the form of "imm(reg)" for load/store instructions.
 *   - Assemble instructions into machine code, supporting R, I, S, B, U, and J instruction formats.
 *   - Maintain a list of assembled instructions and provide accessors for labels, instructions,
 *     and source line mappings.
 *   - Reset its internal state for reuse.
 *   - Print the original assembly code for debugging or display purposes.
 *
 * Key Methods:
 *   - getLabelSet(): Returns the set of collected labels.
 *   - getInstructions(): Returns the list of assembled instruction instances.
 *   - reset(): Clears the assembler's internal state.
 *   - getSourceLineToPC(): Returns the mapping from source line numbers to program counters.
 *   - parseMemoryOperand(): Parses memory operands into immediate and register components.
 *   - parseRegister(): Converts register names (numeric or ABI) to register numbers.
 *   - parseImmediate(): Parses immediate values or resolves label references to PC-relative offsets.
 *   - collectLabels(): First pass to collect labels and return instruction lines without label definitions.
 *   - assemble(): Main method to assemble a vector of assembly lines into machine code.
 *   - printInstructions(): Prints the original assembly code for each instruction.
 *
 * Error Handling:
 *   - Provides error messages for invalid operands, unknown instructions, and unsupported formats.
 *   - Handles undefined labels and invalid register names gracefully.
 *
 * Dependencies:
 *   - Relies on supporting classes such as LabelSet, InstructionSet, InstructionInstance, and a parser utility.
 */
#include "Assembler.h"
#include <sstream>
#include <algorithm>
#include <stdexcept>
#include <iostream>
#include <unordered_set>
#include <iomanip>

// returns the current source line number for a given program counter (pc)
// If the pc is not found, returns 0
int Assembler::getCurrentSourceLine(int pc) const
{
    int sourceLine = 0;
    for (const auto &[sLine, mappedPC] : sourceLineToPC)
    {
        if (mappedPC == pc)
        {
            sourceLine = sLine; // Found the source line corresponding to the PC
        }
    }
    return sourceLine;
}

void Assembler::logError(const std::string &message, int pc) const
{
    int lineNumber = getCurrentSourceLine(pc);
    out << " [Line " << lineNumber << "] " << message << std::endl;
}

const LabelSet &Assembler::getLabelSet() const
{
    return labelSet;
}

// Getter for instructions
const std::vector<InstructionInstance> &Assembler::getInstructions() const
{
    return instructions;
 }

 void Assembler::reset() {
        // Reset the assembler state
        labelSet.clear();
        instructions.clear();
        sourceLineToPC.clear();
        machineCodes.clear();
}

// Getter for sourceLineToInstructionIndex
const std::unordered_map<int, int> &Assembler::getSourceLineToPC() const
{
    return sourceLineToPC;
}

// parseMemoryOperand parses a memory operand like "4(x5)" into immediate and register
bool Assembler::parseMemoryOperand(const std::string &operand, int &imm, int &rs1, int pc) const
{
    size_t openParen = operand.find('(');
    size_t closeParen = operand.find(')');
    if (openParen == std::string::npos || closeParen == std::string::npos || closeParen <= openParen + 1)
    {
        logError("Invalid memory operand format: " + operand, pc);
        return false;
    }

    std::string immPart = operand.substr(0, openParen);
    std::string regPart = operand.substr(openParen + 1, closeParen - openParen - 1);

    imm = parseImmediate(immPart, pc);
    rs1 = parseRegister(regPart);

    return true;
}

// parseRegister converts register name like "x5" to integer 5
// Does not support a0, a1, a2, a3, a4, a5, a6, a7 etc.
int Assembler::parseRegister(const std::string &token) const
{

    // Using hardcoded register names to support ABI names
    static const std::unordered_map<std::string, int> registerMap = {
        // Numeric form
        {"x0", 0},
        {"x1", 1},
        {"x2", 2},
        {"x3", 3},
        {"x4", 4},
        {"x5", 5},
        {"x6", 6},
        {"x7", 7},
        {"x8", 8},
        {"x9", 9},
        {"x10", 10},
        {"x11", 11},
        {"x12", 12},
        {"x13", 13},
        {"x14", 14},
        {"x15", 15},
        {"x16", 16},
        {"x17", 17},
        {"x18", 18},
        {"x19", 19},
        {"x20", 20},
        {"x21", 21},
        {"x22", 22},
        {"x23", 23},
        {"x24", 24},
        {"x25", 25},
        {"x26", 26},
        {"x27", 27},
        {"x28", 28},
        {"x29", 29},
        {"x30", 30},
        {"x31", 31},

        // ABI names
        {"zero", 0},
        {"ra", 1},
        {"sp", 2},
        {"gp", 3},
        {"tp", 4},
        {"t0", 5},
        {"t1", 6},
        {"t2", 7},
        {"s0", 8},
        {"fp", 8},
        {"s1", 9},
        {"a0", 10},
        {"a1", 11},
        {"a2", 12},
        {"a3", 13},
        {"a4", 14},
        {"a5", 15},
        {"a6", 16},
        {"a7", 17},
        {"s2", 18},
        {"s3", 19},
        {"s4", 20},
        {"s5", 21},
        {"s6", 22},
        {"s7", 23},
        {"s8", 24},
        {"s9", 25},
        {"s10", 26},
        {"s11", 27},
        {"t3", 28},
        {"t4", 29},
        {"t5", 30},
        {"t6", 31}};

    auto it = registerMap.find(token);
    if (it != registerMap.end())
    {
        return it->second;
    }

    out << "Invalid register: " << token << std::endl;
    return -1;
}

int32_t Assembler::parseImmediate(const std::string &token, int pc) const
{
    std::string trimmedToken = parser.trim(token);
    // Check if the token is a label
    if (labelSet.isPresent(trimmedToken))
    {
        return labelSet.getProgramCounter(trimmedToken) - pc; // PC-relative
    }

    // If not a label, try to parse as an immediate value
    // std::stoi will auto-detect hex (0x), octal (0), or decimal if base is 0
    try
    {
        return std::stoi(trimmedToken, nullptr, 0);
    }
    catch (const std::invalid_argument &)
    {
        logError("Undefined label:" + token, pc);
        return 0;
    }
}

// collectLabels collects labels in LabelSet and returns instruction lines only (with label definitions removed)
std::vector<std::string> Assembler::collectLabels(const std::vector<std::string> &lines)
{
    std::vector<std::string> instructionsOnly;
    sourceLineToPC.clear(); // clear in case reusing the assembler
    // ---------- First pass: collect labels ----------
    int currentInstructionIndex = 0;
    int numLines = lines.size();
    uint32_t baseAddr = 0x0;

    for (int originalLineNum = 0; originalLineNum < numLines; ++originalLineNum)
    {
        std::string trimmed;
        trimmed = parser.cleanLine(lines[originalLineNum]);
        // ignore comment lines
        if (trimmed.empty() || trimmed[0] == '#' || trimmed[0] == ';')
            continue;
        uint32_t pc = baseAddr + currentInstructionIndex * 4;

        size_t colonPos = trimmed.find(':');
        if (colonPos != std::string::npos)
        {
            std::string label = trimmed.substr(0, colonPos);
            labelSet.setLabel(pc, label, true);     // Set label for this PC
            trimmed = trimmed.substr(colonPos + 1); // Remainder of the line
            trimmed = parser.cleanLine(trimmed);    // Clean it again (remove spaces/comments)
        }

        // If instruction remains after label, treat it as instruction
        if (!trimmed.empty())
        {
            instructionsOnly.push_back(trimmed);
            sourceLineToPC[originalLineNum + 1] = pc;
            currentInstructionIndex++;
        }
    }
    return instructionsOnly;
}

// assemble parses assembly lines and returns a vector of encoded instructions
std::vector<uint32_t> Assembler::assemble(const std::vector<std::string> &lines)
{

    //std::vector<uint32_t> machineCodes;
    reset(); // Reset the assembler state    
    // ---------- First pass: collect labels ----------
    std::vector<std::string> instructionsOnly = collectLabels(lines);
    // ---------- Second pass: generate machine code ----------
    int currentLine = 0;
    uint32_t baseAddr = 0x0;

    for (const auto &line : instructionsOnly)
    {

        std::stringstream ss(line);
        std::string mnemonic;
        ss >> mnemonic;
        uint32_t pc = baseAddr + currentLine * 4;
        int invalidInstruction = 0;

        const Instruction *instr = instructionSet.getByMnemonic(mnemonic);
        if (!instr)
        {
            logError("Unknown instruction: " + mnemonic, pc);
            pc = pc - 4;
            invalidInstruction = 1;
            // return if instruction is invalid; otherwise label positions will be wrong
            return machineCodes;
        }

        std::string rest;
        std::getline(ss, rest);
        std::vector<std::string> operands = parser.split(rest, ',');

        int rd = 0, rs1 = 0, rs2 = 0;
        int imm = 0;

        switch (instr->format)
        {

        case InstrFormat::R:
            if (operands.size() != 3)
            {
                logError("Expected 3 operands for R-type instruction: ", pc);
                invalidInstruction = 1;
            }
            rd = parseRegister(operands[0]);
            rs1 = parseRegister(operands[1]);
            rs2 = parseRegister(operands[2]);
            break;

        case InstrFormat::I:

            // For ecall/ebreak, we set imm to 0 for ecall and 1 for ebreak
            // ecall is used for system calls, ebreak is used for debugging
            // both have same opcode, but different semantics
            if (instr->mnemonic == "ecall" || instr->mnemonic == "ebreak")
            {
                imm = (instr->mnemonic == "ecall") ? 0 : 1;
                // Special case: ecall/ebreak take no operands
                if (!operands.empty())
                {
                    logError("ecall/ebreak takes no operands, got " + std::to_string(operands.size()), pc);
                    invalidInstruction = 1;
                }
                break;
            }

            rd = parseRegister(operands[0]);

            if (operands.size() == 2)
            {
                if (instr->isLoadInstruction(instr->mnemonic) || instr->mnemonic == "jalr")
                {
                    // Case: load or jalr with memory operand. lw x1, 4(x2) form
                    if (!parseMemoryOperand(operands[1], imm, rs1, pc))
                    {
                        logError("Invalid memory operand: " + operands[1], pc);
                        invalidInstruction = 1;
                    }
                }
            }
            else if (operands.size() == 3)
            {
                // Case: immediate arithmetic (e.g., addi x1, x2, 10)
                rs1 = parseRegister(operands[1]);
                // DO NOT support lw x1, label(x0)
                imm = parseImmediate(operands[2], pc);
            }
            else
            {

                logError("Expected 2 or 3 operands for I-type instruction: " + instr->mnemonic, pc);
                invalidInstruction = 1;
                break;
            }
            break;

        case InstrFormat::S:
            if (operands.size() != 2)
            {
                logError("Expected 2 operands for S-type instruction: " + instr->mnemonic, pc);
                invalidInstruction = 1;
                break;
            }
            // DO NOT SUPPORT sw x2, label(x0)
            rs2 = parseRegister(operands[0]);
            if (!parseMemoryOperand(operands[1], imm, rs1, pc))
            {
                logError("Invalid memory operand for S-type instruction " + operands[1], pc);
                invalidInstruction = 1;
                break;
            }
            break;

        // SUPPORT BEQ x1, x3, label
        case InstrFormat::B:
            if (operands.size() != 3)
            {
                logError("Expected 3 operands for B-type instruction: " + instr->mnemonic, pc);
                invalidInstruction = 1;
                break;
            }
            rs1 = parseRegister(operands[0]);
            rs2 = parseRegister(operands[1]);
            imm = parseImmediate(operands[2], pc);
            break;

        // do not support labels in U-type instructions
        case InstrFormat::U:
            if (operands.size() != 2)
            {
                logError("Expected 2 operands for U-type instruction: " + instr->mnemonic, pc);
                invalidInstruction = 1;
                break;
            }
            rd = parseRegister(operands[0]);
            imm = parseImmediate(operands[1], pc);
            break;

        case InstrFormat::J:
            if (operands.size() != 2)
            {
                out << "Expected 2 operands for J-type" << std::endl;
                logError("Expected 2 operands for J-type instruction: " + instr->mnemonic, pc);
                invalidInstruction = 1;
                break;
            }

            rd = parseRegister(operands[0]);
            // support jal x1, label
            imm = parseImmediate(operands[1], pc);
            // Branch offsets are relative to PC + 4 (the address of the instruction after the branch).
            //  JAL offsets are relative to the current PC (not PC + 4).

            break;

        default:
            out << "Unsupported format" << std::endl;
            logError("Unsupported instruction format: " + instr->mnemonic, pc);
            invalidInstruction = 1;
            break;
        }


    // If the instruction is valid, skip it
    if (!invalidInstruction) {
        // Create an InstructionInstance object
        InstructionInstance instruction(instr, rd, rs1, rs2, imm, pc);
        // needed in Simulator to show original code
        instruction.originalCode= line;
        instructions.push_back(instruction);
        machineCodes.push_back(instr->getMachineCode(rd, rs1, rs2, imm));
    }
    else {// if instruction is not valid; the position of any remaining labels are incorrect.
        return machineCodes;
    }
    currentLine++;

    }

    return machineCodes;
}

void Assembler::printInstructions() const
{
    for (const auto &instr : instructions)
    {
        out << instr.originalCode << std::endl;
    }
}

void Assembler::printBinInstructions() const
{
     for (const auto& code : machineCodes) {
        out << std::hex << std::setw(8) << std::setfill('0') << code << std::endl;
        out << std::dec;
    }

}


// Helper method to build formatted lines (reused by multiple methods)
std::vector<std::pair<uint32_t, std::string>> Assembler::buildFormattedLines() const {
    std::vector<std::pair<uint32_t, std::string>> allLines;
    
    // Add instructions
    for (size_t i = 0; i < instructions.size(); ++i) {
        const auto& instr = instructions[i];
        const uint32_t machineCode = machineCodes[i];
        
        std::stringstream ss;
        ss << std::hex << instr.pc << ": "
           << std::setw(8) << std::setfill('0') << machineCode << " "
           << instr.originalCode;
        
        allLines.push_back({instr.pc, ss.str()});
    }
    
    // Add labels from labelSet
    const auto& labels = labelSet.getAllLabels();
    for (const auto& labelPair : labels) {
        const uint32_t labelPC = static_cast<uint32_t>(labelPair.first);
        const std::string& labelName = labelPair.second;
        
        std::stringstream ss;
        ss << std::hex << labelPC << ": "
           << std::setw(8) << std::setfill('0') << 0 << " "  // Labels have no machine code
           << labelName << ":";
        
        allLines.push_back({labelPC, ss.str()});
    }
    
    // Sort by PC address
    std::sort(allLines.begin(), allLines.end());
    
    return allLines;
}


// If you need the line number in the FULL formatted assembly (including labels)
int Assembler::getFormattedAssemblyLineNumberByPC(uint32_t pc) const {
    auto allLines = buildFormattedLines();
    
    for (size_t i = 0; i < allLines.size(); ++i) {
        if (allLines[i].first == pc) {
            // If it's a label, find the next instruction at same or next PC
            if (allLines[i].second.find("00000000") != std::string::npos && 
                allLines[i].second.back() == ':') {
                
                // Look for next instruction
                for (size_t j = i + 1; j < allLines.size(); ++j) {
                    if (!(allLines[j].second.find("00000000") != std::string::npos && 
                          allLines[j].second.back() == ':')) {
                        return static_cast<int>(j); // Return instruction line number
                    }
                }
            } else {
                return static_cast<int>(i); // This is an instruction
            }
        }
    }
    
    
    return -1; // PC not found
}

// Get all formatted lines as a vector
std::vector<std::string> Assembler::getAllFormattedLines() const {
    auto allLines = buildFormattedLines();
    
    std::vector<std::string> formattedLines;
    formattedLines.reserve(allLines.size());
    
    for (const auto& line : allLines) {
        formattedLines.push_back(line.second);
    }
    
    return formattedLines;
}

void Assembler::printFormattedAssembly() const {
    auto allLines = buildFormattedLines();
    
    // Print all lines in PC order
    for (const auto& line : allLines) {
        out << line.second << std::endl;
    }
    
    out << std::dec; // Reset to decimal
}

