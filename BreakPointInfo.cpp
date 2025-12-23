#include "BreakPointInfo.h"
/*
 * @class BreakPointInfo
 * Manages breakpoints for a simulated computer architecture.
 *
 * This class provides methods to set, remove, and query breakpoints based on program counters (PC)
 * and corresponding assembly source lines. It enforces a maximum number of breakpoints and allows
 * mapping between PC addresses and source lines for debugging purposes.
 */

void BreakPointInfo::reset()
{
    breakpointMap.clear();
}


// Get assembly line at the break point of a given PC ; 0 otherwise
int BreakPointInfo::getBreakpointAssemblyLine(int pc) const {

    
    auto it = breakpointMap.find(pc);
    if (it != breakpointMap.end()) {
        return it->second;
    }
    return 0; 
}

    
bool BreakPointInfo::setBreakpoint(int pc, int assemblyLine) {
    if (breakpointMap.size() >= MAX_BREAKPOINTS) {
        out << "Maximum number of breakpoints (" << MAX_BREAKPOINTS << ") reached.\n";
        out << "Current breakpoints:\n";
        for (const auto& [bpc, bline] : breakpointMap) {
            out << "  PC: 0x" << std::hex << bpc << std::dec
                      << " (source line: " << bline << ")\n";
        }
        return false;
    }

    if (breakpointMap.count(pc) == 0) {
        breakpointMap[pc] = assemblyLine;
        return true;
    }

    return false;
}

bool BreakPointInfo::removeBreakpoint(int assemblyLine) {
    for (auto it = breakpointMap.begin(); it != breakpointMap.end(); ++it) {
        if (it->second == assemblyLine) {
            breakpointMap.erase(it);
            return true;
        }
    }
    return false;
}

bool BreakPointInfo::hasBreakpoinaAtpc(int pc) const {
    return breakpointMap.count(pc)>0? true: false;
}

bool BreakPointInfo::hasBreakpoint(int assemblyLine) const {
    for (const auto& [pc, line] : breakpointMap) {
        if (line == assemblyLine) {
            return true;
        }
    }
    return false;
}