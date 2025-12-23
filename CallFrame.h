#ifndef CALLFRAME_H
#define CALLFRAME_H
#include <iomanip>
#include <iostream>
#include <cstdint>

/*
 * @class CallFrame
 * @brief Represents a function call frame in a simulated call stack.
 *
 * Stores information about a function invocation, including the function name,
 * the source code line number, and the return address for the next instruction.
 */
class CallFrame {
    private:
    std::string functionName;
    int sourceLine;
    uint32_t returnAddress;

    public:
        CallFrame(const std::string& funcName, int srcLine, uint32_t retAddr)
            : functionName(funcName), sourceLine(srcLine), returnAddress(retAddr) {}
        uint32_t getReturnAddress() const { return returnAddress; }
        const std::string& getFunctionName() const { return functionName; }
        CallFrame()=default;
        friend std::ostream& operator<<(std::ostream& os, const CallFrame& frame) {
            os << "  "<<frame.getFunctionName() << std::dec << " at line : " << frame.sourceLine << " Next Instruction 0x" << std::hex 
            << std::setw(8)  << std::setfill('0') << frame.getReturnAddress()<< std::dec <<  " \n";
             return os;
        }
        void setReturnAddress(uint32_t addr) { returnAddress = addr; }
        void setFunctionName(const std::string& name) { functionName = name; }
        void setSourceLine(int line) { sourceLine = line; }
};
#endif