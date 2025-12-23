    #include <sstream>
    #include <memory>
    #include "InteractiveSimulator.h"
    #include <emscripten/bind.h>
    using namespace emscripten;
    // This file provides WebAssembly bindings for the RISC-V simulator.
    // It allows loading RISC-V assembly code, running it, stepping through instructions,
    // and inspecting registers, memory, and the call stack from a JavaScript environment.  

    class WASMSimulator {
    private:
        std::istringstream inputBuffer;
        std::ostringstream outputBuffer;
        InteractiveSimulator sim;

    public:
        WASMSimulator()
            : inputBuffer(""), outputBuffer(), sim(inputBuffer, outputBuffer,ISAMode::RV64IM) {}

        int load(const std::string& asmText) {
            
            return sim.loadProgramFromString(asmText);
        }

        std::string run() {
            sim.run();
            return outputBuffer.str();
        }

        int step() {
            return sim.step();
        }

        std::string getRegisters() {
            outputBuffer.str("");
            outputBuffer.clear();
            sim.printRegisters();
            return outputBuffer.str();
        }

        std::string getCode() {
            outputBuffer.str("");
            outputBuffer.clear();
            sim.getAssembler().printInstructions();
            return outputBuffer.str();
        }

        std::string getBinCode() {
            outputBuffer.str("");
            outputBuffer.clear();
            sim.getAssembler().printBinInstructions();
            return outputBuffer.str();
        }

        std::string printFormattedAssembly() {
            outputBuffer.str("");
            outputBuffer.clear();
            sim.getAssembler().printFormattedAssembly();
            return outputBuffer.str();
        }


        int getNextBinLine() {
            
            return sim.getAssembler().getFormattedAssemblyLineNumberByPC(sim.getPC());
            
        }

        std::string readMemory(uint64_t addr, int count) {
            outputBuffer.str("");
            outputBuffer.clear();
            sim.printMemory(addr, count);
            return outputBuffer.str();
        }

        std::string getStack() {
            outputBuffer.str("");
            outputBuffer.clear();
            sim.showStack();
            return outputBuffer.str();
        }

        void setBreakpoint(int line) {
            sim.setBreakpoint(line);
        }

        void removeBreakpoint(int line) {
            sim.removeBreakpoint(line);
        }

        std::string getHelpText() const {
            return R"(Available commands:

    load <filename>          Load a RISC-V assembly code file
    show-code                Display the loaded RISC-V assembly instructions
    run                      Run the loaded program
    step                     Execute the next instruction
    regs                     Display register contents
    mem <addr> <count>       Show memory contents
    show-stack               Show the current call stack
    break <line>             Set a breakpoint
    del break <line>         Remove a breakpoint)";
        }
    };

    // Add this dummy function
    void dummy_init() {
    // This ensures the runtime actually initializes and runs.
    }


    EMSCRIPTEN_BINDINGS(riscv_simulator_module) {
        function("dummy_init", &dummy_init);    //ensures runtime initializes
        class_<WASMSimulator>("Simulator")
            .constructor()
            .function("load", &WASMSimulator::load)
            .function("run", &WASMSimulator::run)
            .function("step", &WASMSimulator::step)
            .function("getRegisters", &WASMSimulator::getRegisters)
            .function("getCode", &WASMSimulator::getCode)
            .function("getBinCode", &WASMSimulator::getBinCode)
            .function("getNextBinLine", &WASMSimulator::getNextBinLine)
            .function("printFormattedAssembly", &WASMSimulator::printFormattedAssembly)
            .function("readMemory", &WASMSimulator::readMemory)
            .function("getStack", &WASMSimulator::getStack)
            .function("setBreakpoint", &WASMSimulator::setBreakpoint)
            .function("removeBreakpoint", &WASMSimulator::removeBreakpoint)
            .function("getHelpText", &WASMSimulator::getHelpText);
    }
