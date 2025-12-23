

/**
 * @class BreakPointInfo
 * @brief Manages breakpoints for a program, mapping program counters (PC) to source assembly lines.
 *
 * This class allows setting, removing, and querying breakpoints by either program counter or source line.
 * It supports a maximum of 5 breakpoints and provides utility functions to reset and inspect breakpoints.
 *
 * @note Input and output streams can be customized for interaction.
 */
#ifndef BREAKPOINTINFO_H
#define BREAKPOINTINFO_H

#include <unordered_map>
#include <iostream>

class BreakPointInfo {
public:
    BreakPointInfo(std::istream& in = std::cin, std::ostream& out = std::cout)
        : in(in), out(out) {}
    // Set a breakpoint at the given PC and original source line
    bool setBreakpoint(int pc, int assemblyLine);

    // Remove a breakpoint by source line
    bool removeBreakpoint(int assemblyLine);

    // Check if a breakpoint exists at a given source line
    bool hasBreakpoint(int assemblyLine) const;

    // Check if a breakpoint exists at a given program counter
    bool hasBreakpoinaAtpc(int pc) const;

    // Get assembly line at the break point of a given PC 
    int getBreakpointAssemblyLine(int pc) const;

    // reset
    void reset();

    protected:
        std::istream& in;
        std::ostream& out;
private:
    std::unordered_map<int, int> breakpointMap; // pc → source line
    static constexpr int MAX_BREAKPOINTS = 5;
};

#endif // BREAKPOINTINFO_H
