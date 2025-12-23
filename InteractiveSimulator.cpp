/**
 * @file InteractiveSimulator.cpp
 * @brief Implements the interactive command-line interface for the RISC-V simulator.
 *
 * This file provides the implementation for the InteractiveSimulator class, which allows
 * users to interactively load, run, and debug RISC-V assembly programs. The interface
 * supports commands for loading programs, running and stepping through instructions,
 * inspecting registers and memory, managing breakpoints, and viewing the call stack.
 *
 * Main Functions:
 * - loadProgram: Loads and assembles a RISC-V assembly file.
 * - runInteractive: Provides a REPL (Read-Eval-Print Loop) for user commands.
 *
 * Supported Commands:
 *   - help: Show available commands and usage.
 *   - exit: Exit the simulator.
 *   - load <filename>: Load a RISC-V assembly file.
 *   - show-code: Display loaded assembly instructions.
 *   - run: Execute the loaded program until completion or breakpoint.
 *   - step: Execute the next instruction.
 *   - regs: Display register values.
 *   - mem <addr> <count>: Display memory contents.
 *   - show-stack: Show the current call stack.
 *   - break <line>: Set a breakpoint at a source line.
 *   - del break <line>: Remove a breakpoint.
 *
 * Error handling and user guidance are provided for invalid commands and arguments.
 */
#include "InteractiveSimulator.h"
#include "Simulator.h"
#include "Assembler.h"
#include <iostream>
#include <string>
#include <sstream>
#include <fstream>
#include <unordered_map>
#include <set>
#include <vector>
#include <iomanip>
#include <filesystem>

// In InteractiveSimulator.cpp
int InteractiveSimulator::loadProgramFromString(const std::string &asmText)
{

    std::istringstream inStream(asmText);
    assembler.reset(); // Reset the assembler state
    std::vector<std::string> instructions;
    std::string line;
    while (std::getline(inStream, line))
    {
        instructions.push_back(line);
    }

    // Continue with assembling/loading as you would in loadProgram
    std::vector<uint32_t> machineCode = assembler.assemble(instructions);
    if (machineCode.empty())
    {
        out << " Warning: Assembly may have failed — no machine code instructions were generated.\n";
        return false;
    }
    this->load(assembler.getInstructions());

    return assembler.getCurrentSourceLine(0); // Return the first source line to be executed
}

bool InteractiveSimulator::loadProgram(const std::string &asmFilePath)
{
    std::ifstream inFile(asmFilePath);
    if (!inFile)
    {
        out << " Error: Unable to open the assembly input file " << asmFilePath << "'\n";
        return false;
    }
    namespace fs = std::filesystem;
    // Extract filename without extension
    fs::path pathObj(asmFilePath);
    std::string filename = pathObj.stem().string(); // e.g., "program" from "program.asm"
    // Set the program name for cacheSim
    cacheSim.setProgramName(filename);

    std::ostringstream buffer;
    buffer << inFile.rdbuf(); // Read entire file into a string
    loadProgramFromString(buffer.str());

    return true;
}

void InteractiveSimulator::runInteractive()
{
    std::string input;

    /*  \033[1;32m → bright green
        \033[1;31m → bright red
        \033[1;34m → bright blue
        \033[0m → reset */
    out << "\033[1;31mRISSIM> ";

    out << "=== Interactive RISC-V Simulator ===\n";

    while (true)
    {
        out << "RISC Sim> ";
        std::getline(in, input);
        std::istringstream iss(input);
        std::string command;
        iss >> command;

        if (command == "exit")
        {
            out << "Exiting simulator...\n\033[0m";
            reset();
            break;
        }
        else if (command == "load")
        {
            std::string filename;
            iss >> filename;
            if (filename.empty())
            {
                out << "Usage: load <filename>\n";
                continue;
            }
            if (loadProgram(filename))
                out << "Loaded program from " << filename << "\n";
        }
        else if (command == "run")
        {
            run();
            if (cacheSim.isEnabled())
                cacheSim.printStats(out);
        }
        else if (command == "step")
        {
            step();
        }
        else if (command == "regs")
        {
            printRegisters();
        }
        else if (command == "show-code")
        {
            out << "Printing currently loaded instructions .... \n";
            assembler.printInstructions();
        }
        else if (command == "mem")
        {
            std::string addrStr;
            int count;
            iss >> addrStr >> count;
            if (addrStr.empty() || count <= 0)
            {
                out << "Usage: mem <addr> <count>\n";
                continue;
            }
            // Convert address string to integer
            // Supports hex (0x...), octal (0...), or decimal
            uint64_t addr = 0;
            try
            {
                addr = std::stoull(addrStr, nullptr, 0);
            }
            catch (const std::exception &e)
            {
                out << "Invalid address format: " << addrStr << "\n";
                continue;
            }
            printMemory(addr, count);
        }
        else if (command == "show-stack")
        {
            showStack();
        }
        else if (command == "break")
        {
            std::string lineStr;
            iss >> lineStr;
            int line = std::stoi(lineStr);
            setBreakpoint(line);
        }
        else if (command == "del")
        {
            std::string subcmd, lineStr;

            iss >> subcmd >> lineStr;
            int line = std::stoi(lineStr);

            if (subcmd == "break")
            {
                removeBreakpoint(line);
            }
        }
        else if (command == "cache_sim")
        {
            std::string subcmd;
            iss >> subcmd;

            if (subcmd == "enable")
            {
                std::string configFile;
                iss >> configFile;
                if (configFile.empty())
                {
                    out << "Usage: cache_sim enable <config_file>\n";
                    continue;
                }
                if (isProgramLoaded())
                {
                    out << "Cannot enable cache simulation while a program is loaded.\n";
                    continue;
                }
                if (cacheSim.enable(configFile, memory))
                {
                    out << "Cache simulation enabled with configuration from " << configFile << "\n";
                }
                else
                {
                    out << "Failed to enable cache simulation. Check configuration file.\n";
                }
            }
            else if (subcmd == "disable")
            {
                cacheSim.disable();
                out << "Cache simulation disabled.\n";
            }
            else if (subcmd == "status")
            {
                cacheSim.printStatus(out);
            }
            else if (subcmd == "invalidate")
            {
                if (!cacheSim.isEnabled())
                {
                    out << "Cache simulation is not enabled to invalidate cache.\n";
                    continue;
                }
                cacheSim.invalidate();
                out << "Cache invalidated.\n";
            }
            else if (subcmd == "dump")
            {
                std::string dumpFile;
                iss >> dumpFile;
                if (dumpFile.empty())
                {
                    out << "Usage: cache_sim dump <filename>\n";
                    continue;
                }
                if (!cacheSim.isEnabled())
                {
                    out << "Cache simulation is not enabled to dump cache state to a file.\n";
                    continue;
                }
                cacheSim.dump(dumpFile);

                out << "Cache state dumped to " << dumpFile << "\n";
            }
            else if (subcmd == "stats")
            {
                if (!cacheSim.isEnabled())
                {
                    out << "Cache simulation is not enabled to get simulation statistics.\n";
                    continue;
                }
                cacheSim.printStats(out);
            }
            else if (subcmd == "caches")
            {
                if (!cacheSim.isEnabled())
                {
                    out << "Cache simulation is not enabled to get valid cache data.\n";
                    continue;
                }
                cacheSim.dump(out);
            }
            else
            {
                out << "Unknown cache_sim command: " << subcmd << "\n";
            }
        }
        else if (command == "pipeline")
        {
            std::string subcmd;
            iss >> subcmd;

            if (subcmd == "enable")
            {
                hazardDetector.enablePipeline();          
                out << "Pipeline simulation enabled with hazard detection.\n";
                
                
            }
            else if (subcmd == "disable")
            {
               hazardDetector.disablePipeline();
                out << "Pipeline simulation disabled.\n";
                
            }
   
            else if (subcmd == "status")
            {
                hazardDetector.showPipelineStatus();
                
            }

            else if(!hazardDetector.isPipelineEnabled())
            {
                out << "Error: Pipeline simulation is not enabled for analysis.\n";
                continue;
            }

            else if(!isProgramLoaded())
            {
                out << "Error: No program loaded for pipeline analysis.\n";
                continue;
            }

            
            else if (subcmd == "analyze")
            {
                hazardDetector.performanceAnalysis(program, cacheSim.isEnabled() );
                out << "Static analysis of pipeline completed.\n";
            }
            else if (subcmd == "hazards")
            {
                hazardDetector.showHazards();
                
            }
            else if (subcmd == "stalls")
            {
                 hazardDetector.showStallStats();
            }
 
            else if (subcmd == "forwarding")
            {
                std::string subsubcmd;
                iss >> subsubcmd;
                
                if (subsubcmd == "enable")
                {
                    hazardDetector.enableForwarding();
                    out << "Data forwarding enabled.\n";
                }
                else if (subsubcmd == "disable")
                {
                    hazardDetector.disableForwarding();
                    out << "Data forwarding disabled.\n";
                }
                else
                {
                    out << "Usage: pipeline forwarding <enable|disable>\n";
                }
            }
            
            else if (subcmd == "clear")
            {
                hazardDetector.clearPipeline();
                out << "Pipeline state cleared.\n";
            }
            else if (subcmd == "dump")
            {
                std::string dumpFile;
                iss >> dumpFile;
                if (dumpFile.empty())
                {
                    out << "Usage: pipeline dump <filename>\n";
                    continue;
                }
                hazardDetector.dumpPipelineState(dumpFile);
                out << "Pipeline analysis dumped to " << dumpFile << "\n";
            }
            else
            {
                out << "Unknown pipeline command: " << subcmd << "\n";
            }
        }
        else if (input == "help")
        {
            out << "Available commands:\n\n"
                << "  help                             Show this help message\n"
                << "  exit                             Exit the simulator gracefully\n"
                << "  load <filename>                  Load a RISC-V assembly code file (resets memory and registers)\n"
                << "  show-code                        Display the loaded RISC-V assembly instructions with line numbers\n"
                << "  run                              Execute the loaded RISC-V code till completion or breakpoint\n"
                << "  regs                             Display all 64-bit registers in hexadecimal format\n"
                << "  mem <addr> <count>               Display <count> memory values starting at <addr> (data section)\n"
                << "  step                             Execute the next instruction and show: \"Executed <instruction>; PC=<address>\"\n"
                << "  show-stack                       Show the current call stack (function calls and returns)\n"
                << "  break <line>                     Set a breakpoint at a specific source line (up to 5 breakpoints allowed)\n"
                << "  del break <line>                 Remove the breakpoint at the specified line. Shows error if none exists\n"
                << "  cache_sim enable <config_file>   Enable cache simulation with the specified configuration file\n"
                << "  cache_sim disable                Disable cache simulation\n"
                << "  cache_sim status                 Display cache simulation status and configuration\n"
                << "  cache_sim invalidate             Invalidate all cache entries\n"
                << "  cache_sim dump <filename>        Dump current cache entries to the specified file\n"
                << "  cache_sim stats                  Display cache access statistics\n"
                << "  cache_sim caches                 Display all the valid cache detailst\n"
                << "\n"
                << "  Pipeline Simulation Commands:\n"
                << "  pipeline enable                  Activate pipeline simulation with hazard detection\n"
                << "  pipeline analyze                 Perform static analysis of program execution with hazard detection\n"
                << "  pipeline disable                 Turn off pipeline simulation\n"
                << "  pipeline status                  Show current pipeline configuration and state\n"
                << "  pipeline hazards                 Display detected data, control, and structural hazards\n"
                << "  pipeline stalls                  Show statistics on pipeline stalls caused by hazards\n"
                << "  pipeline forwarding <enable|disable>  Control data forwarding to mitigate hazards\n"
                << "  pipeline clear                   Reset the pipeline state\n"
                << "  pipeline dump <filename>         Export pipeline analysis to a file\n"
                << std::endl;
        }
        else
        {
            out << "Unknown command: " << command;
            out << "  Please enter a valid command. Type 'help' to see the list of available commands.\n";
        }
    }
}
