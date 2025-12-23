# RISC-V Assembler/Deassembler Project

This project implements an Assembler, Deassembler and Simulator for RISC-V 32 instructions. It converts human-readable RISC-V assembly code to machine code (binary) and vice versa. The project provides functionality for various instruction formats such as R-type, I-type, S-type, B-type, U-type, and J-type instructions.

## Features

- **Assembler**: Converts assembly code into machine code.
- **Deassembler**: Converts machine code (binary) back into readable assembly code.
- **Simulator**: Simulates the execution of assembly code through an interactive debugger.
- **Instruction Set**: Supports a range of RISC-V instructions.
- **Test Framework**: Includes various test cases to verify the functionality of the assembler, deassembler and simulator under Test directory.

## Files
- `AddressDecoder.h`: Decodes memory addresses into tag, index, and offset components for cache access.
- `Assembler.h/cpp`: Contains the assembler class responsible for converting assembly code to machine code.
- `BitUtils.h`      : Contains utility functions to do bit manipulation
- `BreakPointInfo.h/cpp`: Manages breakpoints for a program, mapping program counters (PC) to source assembly lines.
- `Cache.h/cpp`: Simulates a configurable cache memory system.
- `CacheBlock.h`: A single block (or line) in a CPU cache
- `CacheConfig.h/cpp`: parameters that define how the cache behaves and is structured.
- `CacheSimulator.h/cpp`: Runs a cache simulation, managing the cache behavior and collecting stats
- `CacheSimulatorGUI.py`: Web based visualizer of Cache stats collected for different configurations
- `CallFrame.h`: Class that abstracts a CallFrame for Stack
- `DeAssembler.h/cpp`: Contains the deassembler class responsible for converting machine code back to assembly.
- `ExecutionEngine.h`: Abstract Class that abstracts an execution engine. Simulator implements this
- `Instruction.h/cpp`: Defines the basic instruction for the RISC-V architecture.
- `InstructionSet.h/cpp`: Defines the instruction set for the RISC-V architecture.
- `InstructionInstance.h/cpp`: Defines the instruction instance which corresponds to one instruction in RISC-V architecture.
- `InteractiveSimulator.h/cpp`: Extends the Simulator Class and adds on text based interactive debugging.
- `LabelSet.h/cpp`: Manages the lables for the RISC-V architecture code.
- `Memory.h/cpp`: An abstraction for byte-addressable memory storage.
- `Parser.h/cpp`: Parsing logic for Assembly code.
- `Simulator.h/cpp`: Simulator logic for RISC V instruction set.
- `TestRunner.h/cpp`: Runs all the test cases.
- `main.cpp`: Main program that runs the assembler and deassembler on different instruction formats and test cases.

## Test Files

- `.s test files`: Assembly files (input to assemble and simulator) 
- `.b test files`: binary files (input to deassemble).
- `.expect files`: expected out put for assembler, deasembler and simulator
- `config files` : Used for testing cache simulator


## Web Files
- `index.html`: Web based Simulator layout file (http://localhost:8000/index.html)
- `script.js`: Javascript logic for Web based Simulator
- `serve_wasm.py`: Python file that starts the web server to host web based simulator
- `styles.css`: Style sheet for Web based Simulator which affects the look and feel.
- `Wasm_bindings.cpp`: Bindings to enable Web based Simulator to call C++ from js/html


## How to Run

1. Clone the repository:

   git clone https://github.com/YOUR_USERNAME/RISCV-Assembler-DeAssembler.git

    
2. Compile the code:

   make build
   
3. Test the program:
    make test : runs the test cases given below. 

      1) It will run the assembler on a set of standard instructions and generate machine code.

        This machine code is then sent to the deassembler to get back assembler code.

        One full assembler->deAssembler cycle should get back original instruction.

      2) Run assembler  on a set of .s test files under Test directory. 

        Test output *.b is under Test/assembler and is compared against *.expect in the same directory

      3) Run Interactive Simulator  on a set of .s test files under Test directory. 

        Test output *.out is under Test/simulator and is compared against *.expect in the same directory

      4) Run DeAssembler  on a set of .b test files under Test directory. 

        Test output *.s is under Test/deassembler and is compared against *.expect in the same directory

    NOTE: ALL THE TESTS FROM make test MUST PASS before checking in any changes
    
    make assemble: runs assembler on a file in the current directory. It implements complex number multiplication
      
      runs main.exe -a complexMul.s.      
    
    make deassemble:  runs deassembler on a file in the current directory which implements complex number multiplication
     
      main.exe -d complexMul.b
   Usage: main.exe 
     
         main.exe -t | --test    ------->  Run all tests.
     
         main.exe -a <assembly-file> | --assemble <assembly-file>   ------> Assemble the assembly code file into machine code.
     
         main.exe -d <binary-file> | --disassemble <binary-file>   --------> Disassemble the machine code file into assembly.
    
  4) mingw32-make simulate:  runs simulator in interactive mode
      Available commands:

        help                             Show this help message
        exit                             Exit the simulator gracefully
        load <filename>                  Load a RISC-V assembly code file (resets memory and registers)
        show-code                        Display the loaded RISC-V assembly instructions with line numbers
        run                              Execute the loaded RISC-V code till completion or breakpoint
        regs                             Display all 64-bit registers in hexadecimal format
        mem <addr> <count>               Display <count> memory values starting at <addr> (data section)
        step                             Execute the next instruction and show: "Executed <instruction>; PC=<address>"
        show-stack                       Show the current call stack (function calls and returns)
        break <line>                     Set a breakpoint at a specific source line (up to 5 breakpoints allowed)
        del break <line>                 Remove the breakpoint at the specified line. Shows error if none exists
        cache_sim enable <config_file>   Enable cache simulation with the specified configuration file
        cache_sim disable                Disable cache simulation
        cache_sim status                 Display cache simulation status and configuration
        cache_sim invalidate             Invalidate all cache entries
        cache_sim dump <filename>        Dump current cache entries to the specified file
        cache_sim stats                  Display cache access statistics
        cache_sim caches                 Display all the valid cache detailst
        Pipeline Simulation Commands:
          pipeline enable                  Activate pipeline simulation with hazard detection
          pipeline analyze                 Perform static analysis of program execution with hazard detection
          pipeline disable                 Turn off pipeline simulation
          pipeline status                  Show current pipeline configuration and state
          pipeline hazards                 Display detected data, control, and structural hazards
          pipeline stalls                  Show statistics on pipeline stalls caused by hazards
          pipeline forwarding <enable|disable>  Control data forwarding to mitigate hazards
          pipeline clear                   Reset the pipeline state
          pipeline dump <filename>         Export pipeline analysis to a file


      ./riscv_sim.exe -s
  5) mingw32-make websim:  build the web based simulator

  6) mingw32-make serve: start the webserver to access web based simulator at http://localhost:8000/index.html

  7) streamlit run CachesimulatorGUI.py   : Run the web based Cache Simulation Visualizer.
  
   This  visualization helps you quickly compare how different cache policies and associativity levels impact hit rate.
    
  If cache simulation is enabled, the program generates a file named filename.log where filename is the name of the executed file without the extension, will contain cache simulation data in a format as shown:

  Example log file:
R: Address: 0x1000000, Set: 0x0, Tag: 0x8000, Clean, Miss, Replacing block in set using INVALID_BLOCK
R: Address: 0x1000000, Set: 0x0, Tag: 0x8000, Clean, Miss, Read Allocated Block (WB or WT)
W: Address: 0x1000000, Set: 0x0, Tag: 0x8000, Dirty, Hit, WB
R: Address: 0x1000010, Set: 0x1, Tag: 0x8000, Clean, Miss, Replacing block in set using INVALID_BLOCK
R: Address: 0x1000010, Set: 0x1, Tag: 0x8000, Clean, Miss, Read Allocated Block (WB or WT)
R: Address: 0x1000000, Set: 0x0, Tag: 0x8000, Dirty, Hit
W: Address: 0x1000020, Set: 0x2, Tag: 0x8000, Dirty, Miss, Replacing block in set using INVALID_BLOCK
W: Address: 0x1000020, Set: 0x2, Tag: 0x8000, Dirty, Miss, WB Write-back with Allocation


  One can do multiple runs with different config files and generate files of the format Test/CacheSimulator/Sample.
  The cache configuration is provided as an input file (e.g., cache.config) in the following format:
  SIZE_OF_CACHE (number)
  BLOCK_SIZE (number)
  ASSOCIATIVITY (number)
  REPLACEMENT_POLICY (FIFO or LRU or RANDOM)
  WRITEBACK_POLICY (WB or WT)

  Visualizer summarizes all uploaded configurations, calculating the average hit rate for each.

  You can filter the summary by Write Policy (All, WT for Write-Through, WB for Write-Back).

## Examples
         
         mingw32-make clean  

         mingw32-make build

         mingw32-make test

         mingw32-make websim

         mingw32-make serve

         ./riscv_sim.exe -d factorial.b

         ./riscv_sim.exe -a .\consoleprinting.s

         mingw32-make deassemble

         mingw32-make assemble  

        ./riscv_sim.exe -simulate
 
### 1. **R-format Instructions**
Tests assembly code that uses R-type instructions.

add x3, x4, x7

sub x1, x2, x3

and x8, x9, x10

or x11, x12, x13

xor x14, x15, x16

sll x17, x18, x19

srl x20, x21, x22

sra x23, x24, x25
    
### 2. I-format Instructions
Tests assembly code that uses I-type instructions.

addi x1, x2, 10

andi x3, x4, 15

ori x5, x6, 20

xori x7, x8, 25

slli x9, x10, 2

srli x11, x12, 3

srai x13, x14, 1

lhu x19, 4(x20)

lbu x21, 0(x22)

### 3. S-format Instructions
Tests assembly code that uses S-type instructions.

sw x3, 12(x4)

sh x5, 4(x6)

sb x7, 0(x8)

### 4. B-format Instructions
Tests assembly code that uses B-type instructions.

beq x1, x2, 8

bne x3, x4, 12

blt x5, x6, 16

bge x7, x8, 20

bltu x9, x10, 24

bgeu x11, x12, 28

###  5. U/J-format Instructions
Tests assembly code that uses U-type and J-type instructions.

lui x1, 0x10000

jal x2, 16

jalr x3, 0(x4)

### 6. Tests for label

beq x4, x7, L1

add x1, x2, x3

L1:

addi x5, x6, 12


### 7. Integrated Test: GCD, Factorial, Add/Subtract 10 Numbers
Tests integrated code for computing the GCD, factorial, and sum/difference of numbers.

### 8. Tests provided in the assignment
add x3, x4, x7

beq x4, x7, L1 

add x1, x2, x3

L1:

addi x5, x6, 12

jal x0, L2

sb x5, 12(x6) 

lui x9, 0x10000

L2:

addi x9, x10, 12

### 9. Print Integer assembly

addi x10, x0, -10

addi x11, x0, 10

addi x12, x0, 1

jal x0, printNewLine

printNewLine:

jalr x0, 0(x1)

### Future Work

Assembler:

Full support for ecall, ebreak and any currently missing instructions

Support for the 64-bit instruction set, pseudo-instructions, and RV32I base extensions.

Handle directives such as .section, .global, .data, .text, etc.

Explicit support for stack and recursion management. Users must currently handle memory layout, stack usage, and function calls manually.

Comprehensive syntax error detection and improved error handling.

Assembler/DeAssembler:

Performance optimizations for both the assembler and disassembler.

Robust syntax error detection and clearer error messages.

Configuration options to select between base 32-bit instruction set, 32-bit extensions, and the full 64-bit instruction set.

Simulator:

The line number shown with labels is the last line number executed. So initially, program should be at main:0

Enable loading of binary files through the interactive simulator interface.

Web Simulator:

Add support for setting and removing breakpoints.

Introduce a memory view panel.

Introduce a Cache view panel.

Provide side-by-side views of assembly and corresponding machine code.

Allow inline editing of code within the interface.

Load default view with sample code on startup.

Improve syntax error testing and handling.

Make UI panels resizable for better user experience.

Enable loading of machine code with configuration for base 32-bit, 32-bit extension, or 64-bit instruction sets.

Add a toggle to display register values in either hexadecimal or decimal format.
#   C o m p _ A r c h _ P r o j e c t s  
 