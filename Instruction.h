#ifndef INSTRUCTION_H
#define INSTRUCTION_H

#include <cstdint>
#include <string>
#include <sstream>

enum class InstrFormat
{
    R,
    I,
    S,
    B,
    U,
    J
};

class Instruction
{

public:
    std::string mnemonic;
    InstrFormat format;
    int opcode, funct3, funct7;

    Instruction() = default;
    Instruction(const std::string &mnemonic, InstrFormat format, int opcode, int funct3 = -1, int funct7 = -1)
        : mnemonic(mnemonic), format(format), opcode(opcode), funct3(funct3), funct7(funct7) {}

    // Helper function to extract registers and immediate from machine code
    void decodeRegistersAndImmediate(uint32_t code, int &rd, int &rs1, int &rs2, int &imm) const;

    // Function to get machine code from operands
    uint32_t getMachineCode(int rd, int rs1, int rs2, int imm) const;

    // check if the instruction is a load instruction
    // e.g., lw, lb, lbu, lh, lhu, ld
    bool isLoadInstruction(const std::string &mnemonic) const;
    bool isShiftInstruction(const std::string &mnemonic) const;
    bool isStoreInstruction(const std::string &mnemonic) const;
    bool isBranchInstruction(const std::string &mnemonic) const;
    bool isJumpInstruction(const std::string &mnemonic) const;
    bool isMulDivInstruction(const std::string &mnemonic) const;
    bool isALUInstruction(const std::string &mnemonic) const;
    bool isSystemInstruction(const std::string &mnemonic) const;

};

#endif // INSTRUCTION_H
