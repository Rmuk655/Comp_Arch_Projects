#include "Assembler.h"
#include "DeAssembler.h"
#include "InstructionSet.h"
#include "TestRunner.h"
#include "InteractiveSimulator.h"

#include <iostream>
#include <iomanip>
#include <fstream>
#include <algorithm>

bool fileExists(const std::string &filename)
{
    std::ifstream file(filename);
    return file.good();
}

void assembleFile(const std::string &asmFilePath, ISAMode mode = ISAMode::RV32I)
{
    std::ifstream inFile(asmFilePath);
    if (!inFile)
    {
        std::cerr << "Error: Could not open assembly input file '" << asmFilePath << "'\n";
        return;
    }

    std::vector<std::string> instructions;
    std::string line;
    while (std::getline(inFile, line))
    {

        // if (!line.empty())
        instructions.push_back(line);
    }

    Assembler assembler(std::cin, std::cout, mode);
    std::vector<uint32_t> machineCode = assembler.assemble(instructions);

    std::cout << "Assembled machine code:\n";
    for (const auto &code : machineCode)
    {
        std::cout << std::hex << std::setw(8) << std::setfill('0') << code << std::endl;
        std::cout << std::dec;
    }
}

void disassembleFile(const std::string &binFilePath, ISAMode mode = ISAMode::RV32I)
{
    std::ifstream inFile(binFilePath, std::ios::binary);
    if (!inFile)
    {
        std::cerr << "Error: Could not open machine code input file '" << binFilePath << "'\n";
        return;
    }

    std::cout << "Disassembling file: " << binFilePath << "\n";
    // Read binary machine code, convert to assembly, write output (e.g., to binFilePath + ".asm")
    DeAssembler deassembler(std::cin, std::cout, mode);
    std::vector<uint32_t> machineCodes;
    std::string line;

    while (std::getline(inFile, line))
    {
        // Trim whitespace
        line.erase(0, line.find_first_not_of(" \t\r\n"));
        line.erase(line.find_last_not_of(" \t\r\n") + 1);
        // Remove non-hex characters
        line.erase(std::remove_if(line.begin(), line.end(), [](char c)
                                  {
                                      return !std::isxdigit(c); // only keep 0–9, a–f, A–F
                                  }),
                   line.end());

        if (!line.empty())
        {
            try
            {

                // Validate that string contains only hex digits
                if (line.find_first_not_of("0123456789abcdefABCDEF") != std::string::npos)
                {
                    throw std::invalid_argument("Non-hex character in line");
                }

                // Check length (max 8 hex digits for 32-bit)
                if (line.length() > 8)
                {
                    throw std::out_of_range("Line too long for 32-bit value");
                }
                // Assuming machine code is in hex (e.g., "0x00050293" or "00050293")
                uint32_t code = static_cast<uint32_t>(std::stoul(line, nullptr, 16));
                machineCodes.push_back(code);
            }
            catch (const std::exception &e)
            {
                std::cerr << "Invalid machine code line: " << line << " (" << e.what() << ")\n";
            }
        }
    }
    std::vector<std::string> disassembled = deassembler.disassemble(machineCodes);
    std::cout << "\nDisassembled Output:\n";
    for (const auto &line : disassembled)
    {
        std::cout << line << "\n";
    }
    return;
}

void printUsage(const std::string &programName)
{
    std::cout << "Usage:\n"
              << "  " << programName << " [-mode <mode>] -t | --test\n"
              << "      Run all tests.\n\n"
              << "  " << programName << " [-mode <mode>] -a <assembly-file> | --assemble <assembly-file>\n"
              << "      Assemble the assembly code file into machine code.\n\n"
              << "  " << programName << " [-mode <mode>] -d <binary-file> | --disassemble <binary-file>\n"
              << "      Disassemble the machine code file into assembly.\n\n"
              << "  " << programName << " [-mode <mode>] -s | --simulate\n"
              << "      Launch the simulator in interactive mode.\n\n";
}

int main(int argc, char *argv[])
{

    std::string modeStr = "RV64IM"; // default mode
    int argIndex = 1;

    if (argc >= 3 && std::string(argv[1]) == "-mode")
    {
        modeStr = argv[2];
        argIndex = 3; // skip mode argument for the rest of parsing
    }

    if (argc - argIndex < 1)
    {
        printUsage(argv[0]);
        return 1;
    }

    std::string arg1 = argv[argIndex];

    ISAMode mode = stringToIsaMode(modeStr); // Ensure mode is valid
    if (arg1 == "-t" || arg1 == "--test")
    {
        TestRunner testRunner;
        int result = testRunner.runAllTests(mode);
        return result;
    }
    else if ((arg1 == "-a" || arg1 == "--assemble") && argc - argIndex >= 2)
    {
        std::string asmFile = argv[argIndex + 1];

        if (!fileExists(asmFile))
        {
            std::cerr << "Error: Assembly file '" << asmFile << "' does not exist.\n";
            return 1;
        }
        assembleFile(asmFile, mode);
        return 0;
    }
    else if ((arg1 == "-d" || arg1 == "--disassemble") && argc - argIndex >= 2)
    {
        std::string binFile = argv[argIndex + 1];
        if (!fileExists(binFile))
        {
            std::cerr << "Error: Binary file '" << binFile << "' does not exist.\n";
            return 1;
        }
        disassembleFile(binFile, mode);
        return 0;
    }
    else if (arg1 == "-s" || arg1 == "--simulate")
    {
        std::cout << "Launching simulator in interactive mode...\n";
        InteractiveSimulator commlineSimulator(std::cin, std::cout, mode);
        commlineSimulator.runInteractive();

        return 0;
    }

    else
    {
        std::cerr << "Invalid or missing option.\n\n";
        printUsage(argv[0]);
        return 1;
    }

    return 0;
}
