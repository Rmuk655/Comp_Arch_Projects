#ifndef DEASSEMBLER_H
#define DEASSEMBLER_H
#include <string>
#include <vector>
#include <iostream>
#include "InstructionSet.h"
#include "LabelSet.h"

class DeAssembler
{
public:
    DeAssembler(std::istream &in = std::cin, std::ostream &out = std::cout, ISAMode mode = ISAMode::RV32I)
        : in(in), out(out), labelSet(), instructionSet(mode) {}

    std::vector<std::string> disassemble(const std::vector<uint32_t> &machineCodes);
    void setLabelSet(const LabelSet &labelSet);

protected:
    std::istream &in;
    std::ostream &out;

private:
    LabelSet labelSet;
    const InstructionSet instructionSet;
    std::string formatOperands(const Instruction &instr, int rd, int rs1, int rs2, int imm, uint32_t pc);
    void logError(const std::string &message, int lineNum = -1) const;
};

#endif
