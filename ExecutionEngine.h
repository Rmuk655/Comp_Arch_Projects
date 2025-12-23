/**
 * @class ExecutionEngine
 * @brief Abstract base class representing a generic execution engine for processing instructions.
 *
 * This class defines the interface for an execution engine capable of loading, running,
 * stepping through, and resetting a sequence of instructions. It also provides methods
 * for inspecting registers, memory, and the stack, as well as managing breakpoints.
 */
#pragma once
#ifndef _EXECUTION_ENGINE_H_
#define _EXECUTION_ENGINE_H_
#include "InstructionInstance.h"
#include <vector>
#include <string>


class ExecutionEngine {
public:
    virtual void load(const std::vector<InstructionInstance>& instructions) = 0;
    virtual void run() = 0;
    virtual int step() = 0;
    virtual void reset() = 0;
    virtual void printRegisters() const = 0;
    virtual void printMemory(uint32_t address, uint32_t count) const = 0;
    virtual bool setBreakpoint(int line) = 0;
    virtual bool removeBreakpoint(int line) = 0;
    virtual void showStack()  = 0;
    virtual ~ExecutionEngine() = default;
};
#endif
