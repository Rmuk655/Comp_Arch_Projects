/**
 * @class InstructionInstance
 * @brief Represents a specific instance of an instruction with its operands and immediate value.
 *
 * This class encapsulates a decoded instruction, including its destination and source registers,
 * immediate value, and the original code string. It holds a pointer to the corresponding
 * Instruction definition.
 *
 * Members:
 * - instruction: Pointer to the Instruction definition.
 * - rd: Destination register index.
 * - rs1: First source register index.
 * - rs2: Second source register index.
 * - imm: Immediate value associated with the instruction.
 * - originalCode: The original instruction code as a string.
 *
 * Constructor:
 * - InstructionInstance(const Instruction* instr, int rd, int rs1, int rs2, int imm)
 *   Initializes an instance with the given instruction pointer, register indices, and immediate value.
 *
 * Friend Functions:
 * - operator<<: Overloads the stream insertion operator for printing the instruction instance.
 */
#ifndef INSTRUCTION_INSTANCE_H
#define INSTRUCTION_INSTANCE_H
#include "Instruction.h"

class InstructionInstance {
public:
    const Instruction* instruction;
    int rd, rs1, rs2;
    int imm;
    std::string originalCode;
    uint32_t pc;  

    InstructionInstance(const Instruction* instr, int rd, int rs1, int rs2, int imm,uint32_t pc)
        : instruction(instr), rd(rd), rs1(rs1), rs2(rs2), imm(imm),pc(pc) {}
    friend std::ostream& operator<<(std::ostream& os, const InstructionInstance& inst) {
        os << inst.originalCode;
        return os;
 
    }
    

};

#endif