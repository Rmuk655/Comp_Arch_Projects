
/**
 * @file InstructionSet.cpp
 * @brief Implementation of the InstructionSet class for representing and querying a RISC-V-like instruction set.
 *
 * This file defines the InstructionSet constructor and member functions for managing a list of supported instructions.
 * Each instruction is described by its mnemonic, format (R, I, S, B, U, J), opcode, funct3, and optionally funct7.
 *
 * Main functionalities:
 * - Initializes a list of supported instructions with their encoding details.
 * - Provides lookup by mnemonic via getByMnemonic().
 * - Provides lookup by encoded 32-bit instruction via getByEncoded().
 *
 * Key Classes and Methods:
 * - InstructionSet::InstructionSet(): Initializes the instruction list and mnemonic map.
 * - const Instruction* InstructionSet::getByMnemonic(const std::string& name) const:
 *      Returns a pointer to the Instruction matching the given mnemonic, or nullptr if not found.
 * - const Instruction* InstructionSet::getByEncoded(uint32_t code) const:
 *      Decodes the given 32-bit instruction and returns a pointer to the matching Instruction, or nullptr if not found.
 *
 * Note:
 * - The matching logic for getByEncoded() handles special cases for shift instructions and different instruction formats.
 * - The extract() utility function is assumed to extract bitfields from the encoded instruction.
 */
#include "InstructionSet.h"
#include "BitUtils.h"
#include <iostream>

using namespace BitUtils;

std::string isaModeToString(ISAMode mode)
{
    switch (mode)
    {
    case ISAMode::RV32I:
        return "RV32I";
    case ISAMode::RV32IM:
        return "RV32IM";
    case ISAMode::RV64I:
        return "RV64I";
    case ISAMode::RV64IM:
        return "RV64IM";
    default:
        return "unknown";
    }
}

ISAMode stringToIsaMode(const std::string &str)
{
    if (str == "RV32I")
        return ISAMode::RV32I;
    if (str == "RV32IM")
        return ISAMode::RV32IM;
    if (str == "RV64I")
        return ISAMode::RV64I;
    if (str == "RV64IM")
        return ISAMode::RV64IM;
    throw std::invalid_argument("Invalid ISA mode: " + str);
}

// List of all instructions in the instruction set
// Each instruction is represented by its mnemonic, format, opcode, funct3, and funct7
InstructionSet::InstructionSet(ISAMode mode)
{
    instructionList = {
        // R-type
        Instruction("add", InstrFormat::R, 0b0110011, 0b000, 0b0000000),
        Instruction("sub", InstrFormat::R, 0b0110011, 0b000, 0b0100000),
        Instruction("and", InstrFormat::R, 0b0110011, 0b111, 0b0000000),
        Instruction("or", InstrFormat::R, 0b0110011, 0b110, 0b0000000),
        Instruction("xor", InstrFormat::R, 0b0110011, 0b100, 0b0000000),
        Instruction("sll", InstrFormat::R, 0b0110011, 0b001, 0b0000000),
        Instruction("srl", InstrFormat::R, 0b0110011, 0b101, 0b0000000),
        Instruction("sra", InstrFormat::R, 0b0110011, 0b101, 0b0100000),

        // I-type (arithmetic)
        Instruction("addi", InstrFormat::I, 0b0010011, 0b000, 0),
        Instruction("andi", InstrFormat::I, 0b0010011, 0b111, 0),
        Instruction("ori", InstrFormat::I, 0b0010011, 0b110, 0),
        Instruction("xori", InstrFormat::I, 0b0010011, 0b100, 0),
        Instruction("slli", InstrFormat::I, 0b0010011, 0b001, 0b0000000),
        Instruction("srai", InstrFormat::I, 0b0010011, 0b101, 0b0100000),
        Instruction("srli", InstrFormat::I, 0b0010011, 0b101, 0b0000000),

        // I-type (loads)
        Instruction("ld", InstrFormat::I, 0b0000011, 0b011, 0),
        Instruction("lw", InstrFormat::I, 0b0000011, 0b010, 0),
        Instruction("lh", InstrFormat::I, 0b0000011, 0b001, 0),
        Instruction("lb", InstrFormat::I, 0b0000011, 0b000, 0),
        Instruction("lhu", InstrFormat::I, 0b0000011, 0b101, 0),
        Instruction("lbu", InstrFormat::I, 0b0000011, 0b100, 0),

        // I-type (jump)
        Instruction("jalr", InstrFormat::I, 0b1100111, 0b000, 0),

        // I-type (system)
        Instruction("ecall", InstrFormat::I, 0b1110011, 0b000, 0x000),  // 0b000, 0, 0, 0x000,   imm = 0x000
        Instruction("ebreak", InstrFormat::I, 0b1110011, 0b000, 0x001), // 0b000, 0, 0, 0x001,   imm = 0x001

        // S-type (stores)
        Instruction("sb", InstrFormat::S, 0b0100011, 0b000, 0),
        Instruction("sh", InstrFormat::S, 0b0100011, 0b001, 0),
        Instruction("sw", InstrFormat::S, 0b0100011, 0b010, 0),

        // B-type (branches)
        Instruction("beq", InstrFormat::B, 0b1100011, 0b000, 0),
        Instruction("bne", InstrFormat::B, 0b1100011, 0b001, 0),
        Instruction("blt", InstrFormat::B, 0b1100011, 0b100, 0),
        Instruction("bge", InstrFormat::B, 0b1100011, 0b101, 0),
        Instruction("bltu", InstrFormat::B, 0b1100011, 0b110, 0),
        Instruction("bgeu", InstrFormat::B, 0b1100011, 0b111, 0),

        // U-type (no funct3/funct7 needed — use zero)
        Instruction("lui", InstrFormat::U, 0b0110111, 0, 0),
        Instruction("auipc", InstrFormat::U, 0b0010111, 0, 0),

        // J-type (no funct3/funct7)
        Instruction("jal", InstrFormat::J, 0b1101111, 0, 0)};

    // Complete RV32M Extension (Multiply/Divide)
    if (mode == ISAMode::RV32IM || mode == ISAMode::RV64I || mode == ISAMode::RV64IM)
    {
        instructionList.insert(instructionList.end(), {                                                                 // R type instructions with multiplication
                                                       Instruction("mul", InstrFormat::R, 0b0110011, 0b000, 0b0000001), // Multiplication
                                                       Instruction("mulh", InstrFormat::R, 0b0110011, 0b001, 0b0000001),
                                                       Instruction("mulhsu", InstrFormat::R, 0b0110011, 0b010, 0b0000001),
                                                       Instruction("mulhu", InstrFormat::R, 0b0110011, 0b011, 0b0000001),
                                                       Instruction("div", InstrFormat::R, 0b0110011, 0b100, 0b0000001),
                                                       Instruction("divu", InstrFormat::R, 0b0110011, 0b101, 0b0000001),
                                                       Instruction("rem", InstrFormat::R, 0b0110011, 0b110, 0b0000001),
                                                       Instruction("remu", InstrFormat::R, 0b0110011, 0b111, 0b0000001)});
    }
    if (mode == ISAMode::RV64I || mode == ISAMode::RV64IM)
    {
        instructionList.insert(instructionList.end(), {Instruction("addw", InstrFormat::R, 0b0111011, 0b000, 0b0000000),
                                                       Instruction("subw", InstrFormat::R, 0b0111011, 0b000, 0b0100000),
                                                       Instruction("sllw", InstrFormat::R, 0b0111011, 0b001, 0b0000000),
                                                       Instruction("srlw", InstrFormat::R, 0b0111011, 0b101, 0b0000000),
                                                       Instruction("sraw", InstrFormat::R, 0b0111011, 0b101, 0b0100000),

                                                       // 64-bit immediate operations
                                                       Instruction("addiw", InstrFormat::I, 0b0011011, 0b000, 0),
                                                       Instruction("slliw", InstrFormat::I, 0b0011011, 0b001, 0b0000000),
                                                       Instruction("srliw", InstrFormat::I, 0b0011011, 0b101, 0b0000000),
                                                       Instruction("sraiw", InstrFormat::I, 0b0011011, 0b101, 0b0100000),

                                                       // 64-bit load/store
                                                       Instruction("ld", InstrFormat::I, 0b0000011, 0b011, 0),
                                                       Instruction("sd", InstrFormat::S, 0b0100011, 0b011, 0),

                                                       // Additional 32-bit loads for 64-bit (sign-extend)
                                                       Instruction("lwu", InstrFormat::I, 0b0000011, 0b110, 0)

                                                      });
    }
    if (mode == ISAMode::RV64IM)
    {
        instructionList.insert(instructionList.end(), {Instruction("mulw", InstrFormat::R, 0b0111011, 0b000, 0b0000001),
                                                       Instruction("divw", InstrFormat::R, 0b0111011, 0b100, 0b0000001),
                                                       Instruction("divuw", InstrFormat::R, 0b0111011, 0b101, 0b0000001),
                                                       Instruction("remw", InstrFormat::R, 0b0111011, 0b110, 0b0000001),
                                                       Instruction("remuw", InstrFormat::R, 0b0111011, 0b111, 0b0000001)});
    }

    for (const auto &instr : instructionList)
    {
        mnemonicMap[instr.mnemonic] = instr;
    }
}

const Instruction *InstructionSet::getByMnemonic(const std::string &name) const
{
    auto it = mnemonicMap.find(name);

    return (it != mnemonicMap.end()) ? &(it->second) : nullptr;
}

const Instruction *InstructionSet::getByEncoded(uint32_t code) const
{

    uint8_t opcode = extract(code, 0, 7);
    uint8_t funct3 = extract(code, 12, 3); // not valid for U and J formats
    uint8_t funct7 = extract(code, 25, 7); // only valid for R and I with srai, srli etc format
    for (const auto &instr : instructionList)
    {
        if (instr.opcode != opcode)
            continue;
        switch (instr.format)
        {
        case InstrFormat::R:
            if (instr.funct3 == funct3 && instr.funct7 == funct7)
                return &instr;
            break;

        case InstrFormat::I:
            if (instr.funct3 == funct3)
            {
                // Special case: shift instructions (srai, srli, slli) have funct7
                if (instr.isShiftInstruction(instr.mnemonic))
                {
                    if (instr.funct7 == funct7)
                        return &instr;
                }
                else
                {
                    // For other I-type instructions, funct7 is not used; so we only check funct3
                    return &instr;
                }
            }
            break;

        case InstrFormat::S:
            if (instr.funct3 == funct3)
                return &instr;
            break;

        case InstrFormat::B:
            if (instr.funct3 == funct3)
                return &instr;
            break;

        case InstrFormat::U:
            return &instr; // only opcode matters
            break;

        case InstrFormat::J:
            return &instr; // only opcode matters
            break;
        }
    }
    return nullptr; // No match
}