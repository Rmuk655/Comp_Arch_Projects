
/**
 * @class ShadowCallStack
 * @brief Manages a shadow call stack for tracking function call and return frames.
 *
 * This class provides mechanisms to push and pop call frames, update frame information,
 * and display the current state of the shadow call stack. It is useful for simulating
 * or analyzing function call behavior in a program, such as for debugging or security purposes.
 */
#include <iostream>
#include <vector>
#include <string>
#include <algorithm>
#include "CallFrame.h"
#include "Assembler.h"


class ShadowCallStack {

    public:
    ShadowCallStack(std::istream& in = std::cin, std::ostream& out = std::cout)
        : in(in), out(out),shadowCallStack() {}

    void reset() ;
        void push_back() ;

        void updateTopFrameSourceLine(int sourceLine) ;

        void updateTopFrameReturnAddress(int pc) ;

        void pushFrame(const Assembler& assembler, int pc) ;
     
        void popFrameIfMatchingReturnAddress(int rd) ;
        
        void showStack() const ;

    protected:
        std::istream& in;
        std::ostream& out;

    private:
        std::vector<CallFrame> shadowCallStack;
        CallFrame currCallFrame;


};
