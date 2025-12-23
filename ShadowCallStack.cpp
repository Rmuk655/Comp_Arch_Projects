/**
 * @file ShadowCallStack.cpp
 * @brief Implementation of the ShadowCallStack class for managing a shadow call stack.
 *
 * This file contains the implementation of the ShadowCallStack class, which is used to
 * maintain a shadow call stack for tracking function calls, return addresses, and source lines
 * during program execution. The stack is manipulated in response to function calls and returns,
 * and provides utilities for debugging and verification of call/return correctness.
 *
 * Functions:
 * - reset(): Clears the shadow call stack.
 * - push_back(): Pushes the current call frame onto the stack.
 * - updateTopFrameSourceLine(int sourceLine): Updates the source line of the top frame.
 * - updateTopFrameReturnAddress(int pc): Updates the return address of the top frame.
 * - pushFrame(const Assembler& assembler, int pc): Pushes a new frame based on assembler info and program counter.
 * - popFrameIfMatchingReturnAddress(int rd): Pops the top frame if the return address matches the expected value.
 * - showStack() const: Displays the current call stack from oldest to newest.
 *
 * Dependencies:
 * - CallFrame.h: Defines the CallFrame class representing a single stack frame.
 * - Assembler.h: Provides assembler context for mapping program counters to source lines and labels.
 * - ShadowCallStack.h: Declares the ShadowCallStack class.
 *
 * Usage:
 * The ShadowCallStack is typically used in environments where call/return integrity needs to be
 * monitored, such as in simulators, debuggers, or security-sensitive applications.
 */

#include <iostream>
#include <vector>
#include <string>
#include <algorithm>
#include "CallFrame.h"
#include "Assembler.h"
#include "ShadowCallStack.h"


    void ShadowCallStack::reset() {
        while (!shadowCallStack.empty()) {
            shadowCallStack.pop_back();
        }
    }


    void ShadowCallStack::push_back() {
        shadowCallStack.push_back(currCallFrame);
    }


    void ShadowCallStack::updateTopFrameSourceLine(int sourceLine) {
        if (!shadowCallStack.empty()) {
            shadowCallStack.back().setSourceLine(sourceLine);
        }
    }

    void ShadowCallStack::updateTopFrameReturnAddress(int pc) {
        if (!shadowCallStack.empty()) {
            shadowCallStack.back().setReturnAddress(pc);
        }
    }


    void ShadowCallStack::pushFrame(const Assembler& assembler, int pc) {
        int sourceLine = -1;
        std::string funcName="";
        

        for (const auto& [sLine, mappedPC] : assembler.getSourceLineToPC()) {
            if (mappedPC == pc) {
                sourceLine = sLine;
                break;
            }
        }

        if (assembler.getLabelSet().isPresent(pc)) {
            funcName = assembler.getLabelSet().getLabel(pc, true);
        }

        // set funcName as main if there is no label
        if((pc==0) && (funcName=="")) funcName ="* main *";
        currCallFrame = CallFrame(funcName, sourceLine, pc);
        shadowCallStack.push_back(currCallFrame);
    }


    void ShadowCallStack::popFrameIfMatchingReturnAddress(int rd) {
        if (shadowCallStack.size() >= 2) {
            CallFrame& secondLast = shadowCallStack[shadowCallStack.size() - 2];
            int shadowReturnAddr = secondLast.getReturnAddress();

            if (shadowReturnAddr == rd) {
                shadowCallStack.pop_back();
                out << "Found a shadowReturnAddr removing it\n";
                showStack();
                
            } else {
                out << "Warning! Shadow call stack mismatch! Expected: 0x" 
                          << std::hex << shadowReturnAddr 
                          << " but asking to pop 0x" << rd << std::dec 
                          << "\n";
                   
                shadowCallStack.pop_back(); // Still popping?
                showStack();
                            }
        } else {
            out << "Shadow call stack empty on jalr!\n";
        }
    }

    void ShadowCallStack::showStack() const {
        std::vector<CallFrame> tempStack = shadowCallStack;
        std::vector<CallFrame> frames;

        while (!tempStack.empty()) {
            frames.push_back(tempStack.back());
            tempStack.pop_back();
        }

        std::reverse(frames.begin(), frames.end());

        out << "Call stack (oldest to newest):\n";
        for (size_t i = 0; i < frames.size(); ++i) {
            out << "#" << i << " " << frames[i];
        }
    }
