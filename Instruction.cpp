/**
 * @file Instruction.cpp
 * @brief Implementation of the Instruction class for RISC-V instruction decoding and encoding.
 *
 * This file provides methods for:
 * - Identifying load instructions.
 * - Decoding RISC-V instruction fields (registers and immediates) from machine code.
 * - Encoding instruction fields into machine code.
 *
 * Functions:
 * - bool Instruction::isLoadInstruction(const std::string& mnemonic) const
 *   Checks if the given mnemonic corresponds to a load instruction (e.g., lw, lb, lbu, lh, lhu, ld, lwu).
 *
 * - void Instruction::decodeRegistersAndImmediate(uint32_t code, int& rd, int& rs1, int& rs2, int& imm) const
 *   Decodes the register indices and immediate value from a 32-bit instruction code based on the instruction format.
 *   Handles R, I, S, B, U, and J formats, including special cases for shift and system instructions.
 *
 * - uint32_t Instruction::getMachineCode(int rd, int rs1, int rs2, int imm) const
 *   Encodes the given register indices and immediate value into a 32-bit machine code instruction based on the format.
 *   Handles all RISC-V instruction formats and special cases for shift and system instructions.
 *
 * Helper functions (assumed to be defined elsewhere):
 * - int extract(uint32_t value, int start, int length): Extracts a bitfield from the value.
 * - uint32_t pack(uint32_t value, int start, int length): Packs a value into a bitfield at the specified position.
 *
 * @note This implementation assumes the existence of the InstrFormat enum, and member variables such as format, funct3, funct7, opcode, and mnemonic.
 */

#include "Instruction.h"
#include "BitUtils.h"
#include <iomanip>
#include <unordered_set>
using namespace BitUtils;

// Check if the instruction is a load instruction
// e.g., lw, lb, lbu, lh, lhu, ld

bool Instruction::isLoadInstruction(const std::string &mnemonic) const
{
    static const std::unordered_set<std::string> loads = {
        "lb", "lh", "lw", "lbu", "lhu", "ld", "lwu"};
    return loads.count(mnemonic) > 0;
}

bool Instruction::isShiftInstruction(const std::string &mnemonic) const
{
    static const std::unordered_set<std::string> shifts = {
        "sll", "srl", "sra", "slli", "srli", "srai", "sllw", "srlw", "sraw", "slliw", "srliw", "sraiw"};
    return shifts.count(mnemonic) > 0;
}



bool Instruction::isStoreInstruction(const std::string &mnemonic) const {
    static const std::unordered_set<std::string> stores = {
        "sb", "sh", "sw", "sd"
    };
    return stores.count(mnemonic) > 0;
}


bool Instruction::isBranchInstruction(const std::string &mnemonic) const {
    static const std::unordered_set<std::string> branches = {
        "beq", "bne", "blt", "bge", "bltu", "bgeu"
    };
    return branches.count(mnemonic) > 0;
}

bool Instruction::isJumpInstruction(const std::string &mnemonic) const {
    static const std::unordered_set<std::string> jumps = {
        "jal", "jalr"
    };
    return jumps.count(mnemonic) > 0;
}

bool Instruction::isMulDivInstruction(const std::string &mnemonic) const {
    static const std::unordered_set<std::string> muldivs = {
        "mul", "mulh", "mulhsu", "mulhu",
        "div", "divu", "rem", "remu",
        "mulw", "divw", "divuw", "remw", "remuw"
    };
    return muldivs.count(mnemonic) > 0;
}

bool Instruction::isALUInstruction(const std::string &mnemonic) const {
    static const std::unordered_set<std::string> alus = {
        "add", "sub", "and", "or", "xor", "slt", "sltu",
        "addi", "andi", "ori", "xori", "slti", "sltiu",
        "addw", "subw", "addiw",
        // Shift instructions are also ALU, so include them
        "sll", "srl", "sra", "slli", "srli", "srai",
        "sllw", "srlw", "sraw", "slliw", "srliw", "sraiw"
    };
    return alus.count(mnemonic) > 0;
}

bool Instruction::isSystemInstruction(const std::string &mnemonic) const {
    static const std::unordered_set<std::string> system = {
        "ecall", "ebreak"
    };
    return system.count(mnemonic) > 0;
}

void Instruction::decodeRegistersAndImmediate(uint32_t code, int &rd, int &rs1, int &rs2, int &imm) const
{

    int funct3, funct7;
    switch (format)
    {

    case InstrFormat::R:
        rd = extract(code, 7, 5);
        rs1 = extract(code, 15, 5);
        rs2 = extract(code, 20, 5);
        imm = 0;
        break;

    case InstrFormat::I:
        rd = extract(code, 7, 5);
        rs1 = extract(code, 15, 5);
        rs2 = 0;

        funct3 = extract(code, 12, 3);
        funct7 = extract(code, 25, 7);

        // Special case for slli, srli, srai: funct3 == 001 or 101
        if (funct3 == 0b001 || funct3 == 0b101)
        {
            // These are shift instructions: extract shamt only
            // shamt is unsigned, use int32_t for uniformity
            imm = extract(code, 20, 5);
        }
        else
        {
            // Sign-extend 12-bit immediate
            imm = static_cast<int32_t>(code) >> 20;
        }

        break;

    case InstrFormat::S:
        rs1 = extract(code, 15, 5);
        rs2 = extract(code, 20, 5);
        imm = (extract(code, 25, 7) << 5) | extract(code, 7, 5);
        if (imm & (1 << 11))
            imm |= 0xFFFFF000; // sign-extend
        rd = 0;
        break;

    case InstrFormat::B:
        rs1 = extract(code, 15, 5);
        rs2 = extract(code, 20, 5);
        imm = (extract(code, 31, 1) << 12) |
              (extract(code, 7, 1) << 11) |
              (extract(code, 25, 6) << 5) |
              (extract(code, 8, 4) << 1);
        if (imm & (1 << 12))
            imm |= 0xFFFFE000; // sign-extend
        rd = 0;
        break;

    case InstrFormat::U:
        rd = extract(code, 7, 5);
        imm = extract(code, 12, 20); // << 12; Commented for : expected: lui x9, 0x10000 Got:      lui x9, 0x10000000
        rs1 = rs2 = 0;
        break;

    case InstrFormat::J:
        rd = extract(code, 7, 5);
        imm = (extract(code, 31, 1) << 20) |
              (extract(code, 12, 8) << 12) |
              (extract(code, 20, 1) << 11) |
              (extract(code, 21, 10) << 1);
        if (imm & (1 << 20))
            imm |= 0xFFF00000; // sign-extend
        rs1 = rs2 = 0;
        break;
    }
}

uint32_t Instruction::getMachineCode(int rd, int rs1, int rs2, int imm) const
{
    uint32_t code = 0;
    switch (format)
    {
    case InstrFormat::R:
        code |= pack(funct7, 25, 7);
        code |= pack(rs2, 20, 5);
        code |= pack(rs1, 15, 5);
        code |= pack(funct3, 12, 3);
        code |= pack(rd, 7, 5);
        code |= pack(opcode, 0, 7);
        break;

    case InstrFormat::I:
        code |= pack(rs1, 15, 5);
        code |= pack(funct3, 12, 3);
        code |= pack(rd, 7, 5);
        code |= pack(opcode, 0, 7);
        // Only apply funct7 for shift instructions like srai, srli, slli
        if (isShiftInstruction(mnemonic))
        {
            uint32_t shamt = imm & 0x1F; // low 5 bits
            code |= pack(shamt, 20, 5);  // bits 20–24
            code |= pack(funct7, 25, 7); // funct7 from Instruction
        }
        else if (mnemonic == "ebreak" || mnemonic == "ecall")
        {
            // These have rd = 0, rs1 = 0, funct3 = 0, opcode = 0x73
            code |= pack(imm, 20, 12); // imm = 1 for ebreak, 0 for ecall
        }
        else
        {
            code |= pack(imm, 20, 12); // regular I-type imm
        }
        break;

    case InstrFormat::S:
        code |= pack((imm >> 5), 25, 7);
        code |= pack(rs2, 20, 5);
        code |= pack(rs1, 15, 5);
        code |= pack(funct3, 12, 3);
        code |= pack(imm, 7, 5);
        code |= pack(opcode, 0, 7);
        break;

    case InstrFormat::B:
        code |= pack((imm >> 12), 31, 1);
        code |= pack((imm >> 5), 25, 6);
        code |= pack(rs2, 20, 5);
        code |= pack(rs1, 15, 5);
        code |= pack(funct3, 12, 3);
        code |= pack((imm >> 1), 8, 4);
        code |= pack((imm >> 11), 7, 1);
        code |= pack(opcode, 0, 7);
        break;

    case InstrFormat::U:
        code |= (imm << 12);
        code |= pack(rd, 7, 5);
        code |= pack(opcode, 0, 7);
        break;

    case InstrFormat::J:
        uint32_t uimm = static_cast<uint32_t>(imm);
        code |= pack((uimm >> 20), 31, 1);
        code |= pack((uimm >> 1), 21, 10);
        code |= pack((uimm >> 11), 20, 1);
        code |= pack((uimm >> 12), 12, 8);
        code |= pack(rd, 7, 5);
        code |= pack(opcode, 0, 7);
        break;
    }
    return code;
}
