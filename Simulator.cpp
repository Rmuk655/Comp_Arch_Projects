/*
 * Simulator.cpp
 *
 * This file implements the Simulator class, which emulates a RISC-V-like CPU architecture.
 * The simulator supports instruction execution, register and memory management, stack handling,
 * breakpoints, and call stack tracking for debugging and educational purposes.
 *
 * Key Features:
 * - Register File: 32 general-purpose registers, with x0 always zero.
 * - Memory Model:
 *      - Text section: 0x0 to 0x10000 (instructions, 4 bytes each)
 *      - Data section: starts at 0x10000
 *      - Stack: starts at 0x50000, grows downward
 * - Instruction Execution: Supports R, I, S, B, U, J-type instructions, including arithmetic,
 *   logic, load/store, branch, jump, and system instructions (ecall, ebreak).
 * - Program Loading and Reset: Ability to load a program, reset registers, memory, and stack.
 * - Breakpoint Management: Set, remove, and check breakpoints by source line.
 * - Call Stack Tracking: Maintains a shadow call stack for function calls and returns.
 * - Debug Output: Prints register and memory states, stack frames, and execution trace.
 *
 * Main Methods:
 * - initializeRegisters(): Initializes all registers to zero.
 * - setRegValue(): Sets a register value, with x0 protection.
 * - load(): Loads a vector of instructions into the simulator.
 * - reset(), resetRegsMemStack(): Resets the simulator state.
 * - run(): Executes the loaded program, handling breakpoints and system calls.
 * - step(): Executes a single instruction, updates PC and call stack.
 * - execute(): Decodes and executes a single instruction instance.
 * - readMemory(), writeMemory(): Handles memory access for different data sizes.
 * - printRegisters(), printMemory(): Outputs current register and memory contents.
 * - setBreakpoint(), removeBreakpoint(): Manages breakpoints by source line.
 * - showStack(): Displays the current call stack.
 *
 * Usage:
 * 1. Load a program using load().
 * 2. Optionally set breakpoints.
 * 3. Run the program with run(), or step through instructions with step().
 * 4. Inspect registers, memory, and stack as needed.
 *
 * Note:
 * - The simulator is designed for educational and debugging purposes, not for high performance.
 * - Some advanced RISC-V features and error handling may be simplified.
 */
#include "Simulator.h"
#include <iostream>
#include <iomanip>
#include <algorithm>

// TBD: The text section spans from 0x0 --> 0x10000. Each instruction is 4 bytes.
// TBD: The data section starts at 0x10000.
//  The stack starts at 0x50000 and grows downward.
int32_t STACK_TOP_ADDRESS = 0x50000;
// ABI Name	Register	Description
// zero	x0	Constant 0
// ra	x1	Return address
// sp	x2	Stack pointer
//  Decrement stack pointer by 4 (assuming 4-byte words) and store reg at the memory address pointed by the stack pointer.

bool Simulator::isProgramLoaded() const
{
    if (program.empty())
        return false;
    int instrIndex = (int)(pc) / 4;
    if (instrIndex >= (int)program.size() || instrIndex < 0)
        return false;
    return true;
}

void Simulator::initializeRegisters()
{
    for (int i = 0; i < 32; ++i)
    {
        regs[i] = 0;
    }
}

void Simulator::setRegValue(int regNum, int value)
{
    // x0 always zero ; do not allow to overwrite
    if (regNum == 0)
        return;
    // ra value is for managing return addresses; track when it is overwritten for debugging
    // if (regNum == 1)  std::cout <<" Overwriting ra value : "<< value << "\n";
    regs[regNum] = value;
}

void Simulator::load(const std::vector<InstructionInstance> &instructions)
{
    out << "Loading program...\n";
    reset();
    program = instructions;
}

void Simulator::reset()
{
    resetRegsMemStack();
    program.clear();
    breakpointInfo.reset();
}

void Simulator::resetRegsMemStack()
{
    pc = 0;
    initializeRegisters();
    memory.clear();
    shadowCallStack.reset();
    cacheSim.invalidate();
}

void Simulator::run()
{
    // run will always start afresh and ignore previous ecall, ebreak.
    running = true;
    // reset any previous break points hit
    int hitBreakpoint = false;
    while (pc < 4 * (int)program.size())
    {
        // if no breakpoint at pc, line =0
        int line = breakpointInfo.getBreakpointAssemblyLine(pc);

        if (line && !resumedFromBreakpoint)
        {
            out << "Execution stopped at breakpoint at line: " << std::dec << line << '\n';
            hitBreakpoint = true;
            resumedFromBreakpoint = true; // next time, we skip this breakpoint once
            break;
        }

        resumedFromBreakpoint = false; // cleared only after stepping
        step();
        if (pauseRequested)
        {
            out << "Execution paused due to ebreak.\n";
            break;
        }
    }

    if (!pauseRequested && running && !hitBreakpoint)
    {
        out << "Program completed (possibly reached end of program).\n";
        // resetRegsMemStack();
        // cacheSim.invalidate();
    }
    // reset
    pauseRequested = false;
}

int Simulator::step()
{

    // instruction index is pc / 4 because each instruction is 4 bytes
    int instrIndex = (int)(pc) / 4;
    if (program.size() == 0 || instrIndex < 0 || instrIndex >= (int)program.size())
    {
        // if(instrIndex>0){
        //     if (instrIndex >= (int)program.size() || instrIndex < 0) {
        out << " Nothing to step: reached end of the progeam \n";
        return 0;
    }
    //}

    // FInd the assembly source code line from pc.

    int sourceLine = -1;
    sourceLine = assembler.getCurrentSourceLine(pc);

    int old_pc = pc;
    out << std::dec;

    if (pc == 0)
        shadowCallStack.pushFrame(assembler, pc);
    else
        shadowCallStack.updateTopFrameSourceLine(sourceLine);

    execute(program[instrIndex]);

    out << "Executed: " << program[instrIndex]
        << " (line: " << sourceLine << ") ; PC = " << " hex: 0x" << std::setfill('0') << std::setw(6)
        << std::hex << old_pc << std::dec << "\n";
    // Advance to nexte step ; Every instruction is 4 bytes
    if (pc <= 4 * (int)program.size() - 4)
        pc += 4; // dont update pc if you are already at last line
                 // add nextInstr to top of the stack
    shadowCallStack.updateTopFrameReturnAddress(pc);
    sourceLine = assembler.getCurrentSourceLine(pc);
    return sourceLine; // return the next source line to be executed for debugger
}

void Simulator::execute(const InstructionInstance &inst)
{

    const std::string &name = inst.instruction->mnemonic;
    if (name == "ecall")
    {
        int exitCode = regs[10]; // a0
        out << "Program exited with code: " << exitCode << "\n";
        // TBD: Implement syscall handling logic. for now simply stopping the program execution
        pc = program.size() * 4;
        running = false; // Stop the simulator (halt)
        return;
    }
    else if (name == "ebreak")
    {
        pauseRequested = true; // mark that a pause is requested
        return;
    }

    int64_t rd = 0, rs1 = 0, rs2 = 0, imm = 0;
    rd = regs[inst.rd];
    rs1 = regs[inst.rs1];
    rs2 = regs[inst.rs2];

    imm = inst.imm;
    int64_t simm = (int64_t)((int32_t)(imm << 20) >> 20);
    // Implement R, I, S, B, U, J instructions based on `name`

    // R-type instructions
    if (name == "add")
        rd = rs1 + rs2;
    else if (name == "sub")
        rd = rs1 - rs2;
    else if (name == "and")
        rd = rs1 & rs2;
    else if (name == "or")
        rd = rs1 | rs2;
    else if (name == "xor")
        rd = rs1 ^ rs2;
    else if (name == "sll")
        rd = rs1 << (rs2 & 0x1F);
    else if (name == "srl")
        rd = rs1 >> (rs2 & 0x1F);
    else if (name == "sra")
        rd = (int32_t)rs1 >> (rs2 & 0x1F);

    // --- RV32M Extension (Multiply/Divide) ---
    else if (name == "mul")
        rd = (int32_t)rs1 * (int32_t)rs2;
    else if (name == "mulh")
        rd = ((__int128_t)(int64_t)rs1 * (__int128_t)(int64_t)rs2) >> 64; // Use 128-bit intermediate for high bits
    else if (name == "mulhsu")
        rd = ((__int128_t)(int64_t)rs1 * (__int128_t)(uint64_t)rs2) >> 64;
    else if (name == "mulhu")
        rd = ((__int128_t)(uint64_t)rs1 * (__int128_t)(uint64_t)rs2) >> 64;
    else if (name == "div")
        if (rs2 == 0)
            rd = -1; // Division by zero
        else if (rs1 == INT64_MIN && rs2 == -1)
            rd = INT64_MIN; // Overflow
        else
            rd = (int64_t)rs1 / (int64_t)rs2;
    else if (name == "divu")
        if (rs2 == 0)
            rd = UINT64_MAX; // Division by zero
        else
            rd = (uint64_t)rs1 / (uint64_t)rs2;
    else if (name == "rem")
        if (rs2 == 0)
            rd = rs1; // Remainder is dividend
        else if (rs1 == INT64_MIN && rs2 == -1)
            rd = 0; // Overflow
        else
            rd = (int64_t)rs1 % (int64_t)rs2;
    else if (name == "remu")
        if (rs2 == 0)
            rd = rs1; // Remainder is dividend
        else
            rd = (uint64_t)rs1 % (uint64_t)rs2;

    // --- RV64I Extension (64-bit Base) ---
    else if (name == "addw")
        rd = (int64_t)(int32_t)(rs1 + rs2);
    else if (name == "subw")
        rd = (int64_t)(int32_t)(rs1 - rs2);
    else if (name == "sllw")
        rd = (int64_t)(int32_t)(rs1 << (rs2 & 0x1F));
    else if (name == "srlw")
        rd = (int64_t)(int32_t)((uint32_t)rs1 >> (rs2 & 0x1F));
    else if (name == "sraw")
        rd = (int64_t)(int32_t)((int32_t)rs1 >> (rs2 & 0x1F));

    else if (name == "ld")
        rd = readMemory(rs1 + imm, MemSize::DoubleWord, false);
    else if (name == "sd")
        writeMemory(rs1 + imm, MemSize::DoubleWord, rs2);

    // --- RV64M Extension (64-bit Multiply/Divide) ---
    else if (name == "mulw")
        rd = (int64_t)(int32_t)(rs1 * rs2);
    else if (name == "divw")
    {
        int32_t dividend = rs1;
        int32_t divisor = rs2;
        if (divisor == 0)
            rd = -1;
        else if (dividend == INT32_MIN && divisor == -1)
            rd = (int64_t)INT32_MIN;
        else
            rd = (int64_t)(dividend / divisor);
    }
    else if (name == "divuw")
    {
        uint32_t dividend = rs1;
        uint32_t divisor = rs2;
        if (divisor == 0)
            rd = -1;
        else
            rd = (int64_t)(int32_t)(dividend / divisor);
    }
    else if (name == "remw")
    {
        int32_t dividend = rs1;
        int32_t divisor = rs2;
        if (divisor == 0)
            rd = (int64_t)dividend;
        else if (dividend == INT32_MIN && divisor == -1)
            rd = 0;
        else
            rd = (int64_t)(dividend % divisor);
    }
    else if (name == "remuw")
    {
        uint32_t dividend = rs1;
        uint32_t divisor = rs2;
        if (divisor == 0)
            rd = (int64_t)(int32_t)dividend;
        else
            rd = (int64_t)(int32_t)(dividend % divisor);
    }

    // I-type instructions
    else if (name == "addi")
        rd = rs1 + imm;
    else if (name == "andi")
        rd = rs1 & imm;
    else if (name == "ori")
        rd = rs1 | imm;
    else if (name == "xori")
        rd = rs1 ^ imm;
    else if (name == "slli")
        rd = rs1 << (imm & 0x3F);
    else if (name == "srli")
        rd = ((uint32_t)rs1) >> (imm & 0x1F); // logical shift right
    else if (name == "srai")
        rd = (int32_t)rs1 >> (imm & 0x1F);

    // --- RV64I Extension (64-bit Base) ---
    else if (name == "addiw")
        rd = (int64_t)(int32_t)(rs1 + simm);
    else if (name == "slliw")
        rd = (int64_t)(int32_t)(((uint32_t)rs1) << (imm & 0x1F));
    else if (name == "srliw")
        rd = (int64_t)(int32_t)((uint32_t)rs1 >> (imm & 0x1F));
    else if (name == "sraiw")
        rd = (int64_t)(int32_t)((int32_t)rs1 >> (imm & 0x1F));

    // Load instructions
    else if (name == "lw")
        rd = readMemory(rs1 + imm, MemSize::Word, false);
    else if (name == "lh")
        rd = (int16_t)readMemory(rs1 + imm, MemSize::HalfWord, false);
    else if (name == "lb")
        rd = (int8_t)readMemory(rs1 + imm, MemSize::Byte, false);
    else if (name == "lhu")
        rd = readMemory(rs1 + imm, MemSize::HalfWord, true);
    else if (name == "lbu")
        rd = readMemory(rs1 + imm, MemSize::Byte, true);

    // 64-bit load/store (assuming MemSize::DoubleWord is defined)
    else if (name == "lwu")
        rd = readMemory(rs1 + imm, MemSize::Word, true); // Load word (32 bits) with zero extension

    // Store instructions
    else if (name == "sw")
        writeMemory(rs1 + imm, MemSize::Word, rs2);
    else if (name == "sh")
        writeMemory(rs1 + imm, MemSize::HalfWord, rs2);
    else if (name == "sb")
        writeMemory(rs1 + imm, MemSize::Byte, rs2);

    // Branch instructions -
    else if (name == "beq" && rs1 == rs2)
        pc += imm - 4; // Subtract 4 because PC will be incremented after instruction
    else if (name == "bne" && rs1 != rs2)
        pc += imm - 4; // Subtract 4 because PC will be incremented after instruction
    else if (name == "blt" && (int32_t)rs1 < (int32_t)rs2)
        pc += imm - 4; // Subtract 4 because PC will be incremented after instruction
    else if (name == "bge" && (int32_t)rs1 >= (int32_t)rs2)
        pc += imm - 4; // Subtract 4 because PC will be incremented after instruction
    else if (name == "bltu" && (uint32_t)rs1 < (uint32_t)rs2)
        pc += imm - 4; // Subtract 4 because PC will be incremented after instruction
    else if (name == "bgeu" && (uint32_t)rs1 >= (uint32_t)rs2)
        pc += imm - 4; // Subtract 4 because PC will be incremented after instruction

    // Jump instructions
    else if (name == "jal")
    {

        // save the next instruction before jumping
        int nextInstr = pc + 4;

        // jump to new instruction
        pc += imm;
        // jal x0, multiply; no link (no return address saved) jumps to multiply
        // for cases like above jal does not overwrite register ; it just jumps to a link

        if (inst.rd != 0)
        {
            // jal x1, factorial  with link (saves return address to x1) and jumps to factorial
            setRegValue(inst.rd, nextInstr); // set ra register (x1)
            // before pushing new call frame, add nextInstr to top of the stack
            shadowCallStack.updateTopFrameReturnAddress(pc);
            // add a new call frame to stack
            shadowCallStack.pushFrame(assembler, pc);
        }
        // Subtract 4 because PC will be incremented after instruction
        pc = pc - 4;
    }
    else if (name == "jalr")
    {

        // Always update PC, regardless of whether it's a call or return
        pc = (rs1 + imm) & ~1; // Clear lowest bit as per RISC-V spec

        // Instruction	    Action	Why
        // jal x1, func	    Push	Standard function call
        // jalr x1, x2, 0	Push	Indirect function call
        // jalr x0, x1, 0	Pop	    Standard return
        // jalr x0, x2, 0	No action	Could be jump or tail call
        // jal x0, label	    No action	Just a jump

        // jalr x0, func, 5 and jalr x0, func, 5 are just jump intructions
        // jalr x12, func, 5 and jalr x11, func, 5 are just jump and return with add to stack intructions

        if (inst.rd == 0)
        {
            // This is a return instruction: jalr x0, x1, 0. pop the shadowstack and check if return address matches reg[inst.rd]
            shadowCallStack.popFrameIfMatchingReturnAddress(rd);
        }

        // The above was for Return: jalr x0, x1, 0
        // Not Return: jalr x1, func, 0 (this is a call)
        // Adjust because PC will be incremented after each instruction in step
        pc = pc - 4;
    }
    else if (name == "lui")
    {
        // Upper immediate instructions
        //  U-type instructions like LUI and AUIPC use this format:
        // [ imm[31:12] | rd | opcode ]: The immediate is 20 bits, but it is placed in bits 31 to 12.
        // When executed, this 20-bit value is shifted left by 12 bits, so:
        // lui x1, 0x10000 means: x1 = 0x10000 << 12 = 0x10000000
        rd = imm << 12;
    }
    else if (name == "auipc")
    {
        rd = pc + (imm << 12);
    }

    // Check if instruction writes to a register
    bool writesToRegister = (name == "add" || name == "sub" || name == "and" || name == "or" || name == "xor" ||
                             name == "sll" || name == "srl" || name == "sra" ||
                             name == "addi" || name == "andi" || name == "ori" || name == "xori" ||
                             name == "slli" || name == "srli" || name == "srai" ||
                             name == "lw" || name == "lh" || name == "lb" || name == "lhu" || name == "lbu" ||
                             /*name == "jal" || name == "jalr" || */ name == "lui" || name == "auipc" ||
                             // RV32M
                             name == "mul" ||
                             name == "mulh" || name == "mulhsu" || name == "mulhu" ||
                             name == "div" || name == "divu" || name == "rem" || name == "remu" ||
                             // RV64I
                             name == "addw" || name == "subw" || name == "sllw" || name == "srlw" || name == "sraw" ||
                             name == "addiw" || name == "slliw" || name == "srliw" || name == "sraiw" ||
                             name == "ld" || name == "lwu" ||
                             // RV64M
                             name == "mulw" || name == "divw" || name == "divuw" || name == "remw" || name == "remuw");

    // Impt: do not write to register for jal and jalr here
    // Update destination register if needed
    if (writesToRegister && inst.rd != 0)
    {
        setRegValue(inst.rd, rd);
    }
}

// Print the contents of the registers
void Simulator::printRegisters() const
{
    for (int i = 0; i < 32; ++i)
    {
        out << std::dec << "x" << i << ": " << std::hex << regs[i] << (((i + 1) % 16 == 0) ? "\n" : "\t") << std::dec;
    }
}

bool Simulator::removeBreakpoint(int assemblyline)
{

    if (assembler.getSourceLineToPC().count(assemblyline) == 0)
    {
        out << "No code exists at line number: " << std::dec << assemblyline << " to delete break point \n";
        return false;
    }
    else if (!breakpointInfo.removeBreakpoint(assemblyline))
    {
        out << "No breakpoint set at line: " << std::dec << assemblyline << "\n";
        return false;
    }
    else
    {
        out << "Breakpoint removed at line " << std::dec << assemblyline << "\n";
        return true;
    }
}

bool Simulator::setBreakpoint(int line)
{

    if (assembler.getSourceLineToPC().count(line) == 0)
    {
        out << "No valid Instruction at line number: " << std::dec << line << "\n";
        return false;
    }
    else if (breakpointInfo.hasBreakpoint(line))
    {
        out << "Breakpoint already set at line " << std::dec << line << "\n";
        return false;
    }
    else
    {
        int pc = assembler.getSourceLineToPC().at(line);
        if (breakpointInfo.setBreakpoint(pc, line))
        {
            out << "Breakpoint set at line " << std::dec << line << "\n";
            return true;
        }
        return false;
    }
}

void Simulator::showStack()
{
    shadowCallStack.showStack();
}

// Cache related
void Simulator::enableCache(const std::string &configFile)
{
    if (!cacheSim.enable(configFile, memory))
    {
        out << "Failed to enable cache with config: " << configFile << "\n";
    }
}

void Simulator::disableCache()
{
    cacheSim.disable();
}

void Simulator::invalidateCache()
{
    cacheSim.invalidate();
}

void Simulator::dumpCache(const std::string &filename) const
{
    cacheSim.dump(filename);
}

void Simulator::printCacheStats() const
{
    cacheSim.printStats(out);
}

int Simulator::readMemory(uint32_t addr, MemSize size, bool isUnsigned)
{
    if (cacheSim.isEnabled())
    {
        return cacheSim.read(addr, size, isUnsigned);
    }
    return memory.read(addr, size, isUnsigned);
}

void Simulator::writeMemory(uint32_t addr, MemSize size, uint32_t val)
{
    if (cacheSim.isEnabled())
    {
        cacheSim.write(addr, size, val);
        return;
    }
    memory.write(addr, size, val);
}

void Simulator::printMemory(uint32_t address, uint32_t count) const
{
    memory.print(address, count);
}


