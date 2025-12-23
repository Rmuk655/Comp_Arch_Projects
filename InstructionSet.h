
/**
 * @class InstructionSet
 * @brief Represents a collection of supported instructions and provides lookup functionality.
 *
 * The InstructionSet class manages a set of instructions, allowing retrieval of instruction
 * metadata by mnemonic or by raw 32-bit encoded value. It maintains an internal mapping
 * from mnemonics to Instruction objects and a list of all instructions.
 *
 * Public Methods:
 * - InstructionSet(): Constructs and initializes the instruction set.
 * - const Instruction* getByMnemonic(const std::string& name) const:
 *      Retrieves a pointer to the Instruction corresponding to the given mnemonic name.
 *      Returns nullptr if the mnemonic is not found.
 * - const Instruction* getByEncoded(uint32_t code) const:
 *      Retrieves a pointer to the Instruction corresponding to the given 32-bit encoded value.
 *      Returns nullptr if no matching instruction is found.
 *
 * Private Members:
 * - mnemonicMap: Maps instruction mnemonics (string) to Instruction objects.
 * - instructionList: Stores all Instruction objects in the set.
 */
#ifndef INSTRUCTION_SET_H
#define INSTRUCTION_SET_H
#pragma once

#include <string>
#include <unordered_map>
#include <vector>
#include "Instruction.h"

enum class ISAMode
{
    RV32I,  // Basic RISC-V 32-bit base integer instructions
    RV32IM, // RV32I + Multiply/Divide + pseudo-instructions and assembler directives
    RV64I,  // Full 64-bit base instruction set
    RV64IM, // RV64I + Multiply/Divide + pseudo-instructions + assembler directives
};

std::string isaModeToString(ISAMode mode);       // just declaration
ISAMode stringToIsaMode(const std::string &str); // just declaration

class InstructionSet
{
public:
    InstructionSet(ISAMode mode);

    // Get instruction metadata from mnemonic
    const Instruction *getByMnemonic(const std::string &name) const;

    // Get instruction metadata from raw 32-bit code
    const Instruction *getByEncoded(uint32_t code) const;

    // Get list of all instructions
    // const std::vector<Instruction>& getAll() const;

private:
    std::unordered_map<std::string, Instruction> mnemonicMap;
    std::vector<Instruction> instructionList;
};

#endif // INSTRUCTION_SET_H
