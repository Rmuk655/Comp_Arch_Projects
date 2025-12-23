
/**
 * @file DeAssembler.cpp
 * @brief Implements the DeAssembler class for converting machine code to assembly instructions.
 *
 * This file provides the implementation for the DeAssembler class, which is responsible for
 * disassembling a vector of 32-bit machine code instructions into their corresponding assembly
 * language representations. It also handles label regeneration for round-trip testing and
 * formats operands according to instruction type.
 */

#include "DeAssembler.h"
#include <sstream>
#include <iomanip>
#include <iostream>


void DeAssembler::logError(const std::string& message, int lineNum) const {
    
    out << " [Line " << lineNum << "] " << message << std::endl;
}

void DeAssembler::setLabelSet(const LabelSet& labelSet) {
    this->labelSet = labelSet;
}

std::vector<std::string> DeAssembler::disassemble(const std::vector<uint32_t>& machineCodes)  {
    std::vector<std::string> assemblyLines;
    
    uint32_t baseAddr = 0x0;

   
    for (size_t i = 0; i < machineCodes.size(); ++i) {
        uint32_t pc = baseAddr + i * 4;
        uint32_t code = machineCodes[i];

        // regenerate labels like "loop:" from the original assembler
        // This is needed for round trip testing
        if(labelSet.isPresent(pc)){
            assemblyLines.push_back(labelSet.getLabel(pc,true) + ":");
        }

        const Instruction* instr = instructionSet.getByEncoded(code);
        if (!instr) {
            logError("Unknown machine code: 0x" + std::to_string(code),(i+1));
            continue; // Skip to next instruction if unknown

        }

        int rd = 0, rs1 = 0, rs2 = 0;
        int imm = 0;

        try {
            instr->decodeRegistersAndImmediate(code, rd, rs1, rs2, imm);
            // Special case. We did not know whether instruction was ebreak or ecall instructionSet.getByEncoded
            // It will pick up ecall as it is the first in the instruction set for both values of imm
            // Overwrite instruction to break if imm value found now says so.
            if(instr->mnemonic == "ecall" && imm == 0x001)
                instr = instructionSet.getByMnemonic("ebreak") ;

        } catch (const std::exception& e) {
            logError("Error decoding instruction 0x" + std::to_string(code) + ": " + e.what(), (i+1));
            continue; // Skip to next instruction if decoding fails
        }

        // Format the instruction            
        std::string formatted = instr->mnemonic + " " + formatOperands(*instr, rd, rs1, rs2, imm, pc);
        assemblyLines.push_back(formatted);
    }

    return assemblyLines;
}

std::string DeAssembler::formatOperands(const Instruction& instr, int rd, int rs1, int rs2, int imm, uint32_t pc)  {
    std::stringstream ss;
    uint32_t targetAddr = pc + imm;

    switch (instr.format) {
        case InstrFormat::R:
            ss << "x" << rd << ", x" << rs1 << ", x" << rs2;
            break;

        case InstrFormat::I:
                // Check for ecall/ebreak special encoding:
            if (instr.mnemonic == "ecall" || instr.mnemonic == "ebreak") {
                // These instructions have no operands
            } else if (instr.isLoadInstruction(instr.mnemonic) || instr.mnemonic == "jalr") {
                // Format: lhu x19, 4(x20) Format: jalr x3, 0(x4)
                // jal support to be added????
                ss << "x" << rd << ", " << imm << "(x" << rs1 << ")";
            }  else {
                // Format: addi x1, x2, 5
                ss << "x" << rd << ", x" << rs1 << ", " << imm;
            }
            break;


        case InstrFormat::S:
            // Format: sw x1, 4(x2)
            ss << "x" << rs2 << ", " << imm << "(x" << rs1 << ")";
            break;

        case InstrFormat::B: {
            // Format: beq x1, x2, 8
            ss << "x" << rs1 << ", x" << rs2 << ", " ;
            if (labelSet.isPresent(targetAddr) )  { 
                 // marked this location as needing a label
                ss << labelSet.getLabel(targetAddr) ;
                } else {
                // Always show immediate offset if not marked
                ss << imm;
                }
                 
            break;
            }

        case InstrFormat::U:
            ss << "x" << rd << ", 0x" <<  std::hex << imm;
            ss << std::dec;
            break;

        case InstrFormat::J: {
            ss << "x" << rd << ", ";
            if (labelSet.isPresent(targetAddr) )  { 
                ss << labelSet.getLabel(targetAddr);
            } else {
                ss << imm;
                }
            
            break;
        }
        default:
            // TBD: We dont know the line number here, so we cannot log it
            logError("Unsupported instruction format for mnemonic: " + instr.mnemonic);
            return "";
    }

    return ss.str();
}
