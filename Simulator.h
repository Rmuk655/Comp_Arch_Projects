#ifndef SIMULATOR_H
#define SIMULATOR_H
#pragma once

#include <unordered_map>
#include <set>
#include <stack>
#include "ExecutionEngine.h"
#include "Assembler.h"
#include "ShadowCallStack.h"
#include "BreakPointInfo.h"
#include "CacheSimulator.h"
#include "PipelineSimulator.h"
#include "Memory.h"
#include "HazardDetector.h"

class Simulator : public ExecutionEngine
{

public:
    Simulator(std::istream &in = std::cin, std::ostream &out = std::cout, ISAMode mode = ISAMode::RV32I)
        : in(in), out(out), assembler(in, out, mode), breakpointInfo(in, out), memory(out), 
        hazardDetector(in,out), shadowCallStack(in, out)  {}
    // load the program into the simulator
    void load(const std::vector<InstructionInstance> &instructions) override;
    // this will run until the program ends or a breakpoint is hit
    void run() override;
    // this will run one instruction and return next line number to be executed
    int step() override;
    // this will reset the simulator
    void reset() override;
    // this will print the registers
    void printRegisters() const override;
    // this will print the memory
    void printMemory(uint32_t address, uint32_t count) const override;
    // this will set a breakpoint at a given instruction line
    // bool setBreakpointInfo(int pc, int assemblyline);
    // this will remove a breakpoint at given line number
    bool removeBreakpoint(int assemblyline) override;
    // check if a breakpoint exists at a given line number
    // bool hasBreakpointInfo(int assemblyline) const;
    // this will print the stack ; this is not implemented yet
    void showStack() override;
    // not meant to be used
    bool setBreakpoint(int line) override;

    void initializeRegisters();

    // track anyone setting special register values like x[0] and x[1]
    void setRegValue(int regNum, int value);

    Assembler &getAssembler()
    {
        return assembler;
    }
    // Does the overall simulation, integrating cache and pipeline simulators.
    // Cache related
    void enableCache(const std::string &configFile);
    void disableCache();
    void invalidateCache();
    void dumpCache(const std::string &filename) const;
    void printCacheStats() const;
    bool isProgramLoaded() const;
    CacheSimulator &getCacheSimulator()
    {
        return cacheSim;
    }

    int getPC() const
    {
        return pc;
    }
    //void enableHazardDetection(bool enable = true, PipelineType type = PIPELINE_5_STAGE);
    //void analyzeHazards() ;
    //void performanceAnalysis() ;
protected:
    std::istream &in;
    std::ostream &out;

    Assembler assembler;

    bool resumedFromBreakpoint = false;
    // break points with  program counter as key and original assembly line as value
    BreakPointInfo breakpointInfo;

    void writeMemory(uint32_t addr, MemSize size, uint32_t val);
    int readMemory(uint32_t addr, MemSize size, bool isUnsigned);

    CacheSimulator cacheSim;

    // simple byte-addressable memory
    Memory memory;

    HazardDetector hazardDetector;

    // list of instructions
    std::vector<InstructionInstance> program;

    
    // this is the offset from base addr of the next instruction to be executed in bytes. pc/4 is instruction number
    int pc = 0;

private:

    
    //bool hazardDetectionEnabled = false;
    // call stack details
    ShadowCallStack shadowCallStack;

    // is the simulator running: this is used to stop the simulator when a breakpoint is hit
    bool running = true;
    // is the simulator paused: this is used to pause the simulator when a breakpoint is hit
    bool pauseRequested = false;

    const int baseAddr = 0x0;


    // registers; RISC-V has 32 registers; x0 is hardwired to 0
    int64_t regs[32] = {0};

    void execute(const InstructionInstance &inst);
    void resetRegsMemStack();

    std::string programName;
};
#endif