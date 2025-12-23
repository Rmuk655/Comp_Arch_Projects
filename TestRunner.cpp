#include "TestRunner.h"
#include <iostream>
#include <iomanip>
#include <cassert>

#include <filesystem>
#include <fstream>

#include <algorithm>
#include <cctype>
namespace fs = std::filesystem;

std::string trimRight(const std::string &s)
{
    return std::string(s.begin(), std::find_if(s.rbegin(), s.rend(),
                                               [](unsigned char ch)
                                               { return !std::isspace(ch); })
                                      .base());
}

bool compareTextFilesIgnoreWhitespace(const std::string &file1, const std::string &file2)
{
    std::ifstream f1(file1), f2(file2);
    if (!f1.is_open() || !f2.is_open())
        return false;

    std::string line1, line2;
    while (true)
    {
        bool b1 = static_cast<bool>(std::getline(f1, line1));
        bool b2 = static_cast<bool>(std::getline(f2, line2));
        if (!b1 && !b2)
            break;
        if (b1 != b2)
            return false;
        if (trimRight(line1) != trimRight(line2))
            return false;
    }
    return true;
}

// Helper function to compare two binary files byte-wise
bool compareBinaryFiles(const std::string &file1, const std::string &file2)
{
    std::ifstream f1(file1, std::ios::binary | std::ios::ate);
    std::ifstream f2(file2, std::ios::binary | std::ios::ate);

    if (!f1.is_open() || !f2.is_open())
        return false;

    if (f1.tellg() != f2.tellg())
        return false;

    f1.seekg(0);
    f2.seekg(0);

    std::istreambuf_iterator<char> begin1(f1), end1;
    std::istreambuf_iterator<char> begin2(f2);

    return std::equal(begin1, end1, begin2);
}

// Normalize line endings before comparison
// Instead of comparing in binary mode, read both files line-by-line,
// strip carriage returns (\r), then compare. This allows you to ignore Windows/Unix line-ending differences.
// (normalized text comparison) — it will work on all platforms.
bool compareTextFilesNormalized(const std::string &file1, const std::string &file2)
{
    std::ifstream f1(file1), f2(file2);
    std::string line1, line2;
    int lineNum = 1;

    while (std::getline(f1, line1))
    {

        if (!std::getline(f2, line2))
        {
            std::cerr << file2 << " ended early at line " << lineNum << "\n";
            return false;
        }

        // Normalize both lines: remove \r, trim spaces
        line1.erase(std::remove(line1.begin(), line1.end(), '\r'), line1.end());
        line2.erase(std::remove(line2.begin(), line2.end(), '\r'), line2.end());

        if (line1 != line2)
        {
            std::cerr << "Line " << lineNum << " mismatch:\n";
            std::cerr << file1 << " : [" << line1 << "]\n";
            std::cerr << file2 << " : [" << line2 << "]\n";
            // Print a simple character diff indicator line
            std::cerr << "        ";
            size_t maxLen = std::max(line1.size(), line2.size());
            for (size_t i = 0; i < maxLen; ++i)
            {
                char c1 = (i < line1.size()) ? line1[i] : ' ';
                char c2 = (i < line2.size()) ? line2[i] : ' ';
                std::cerr << (c1 == c2 ? ' ' : '^');
            }
            std::cerr << "\n";

            return false;
        }

        ++lineNum;
    }

    if (std::getline(f2, line2))
    {
        std::cerr << file1 << " ended early, extra content in File2 starting at line " << lineNum << "\n";
        return false;
    }
    return true; // Ensure both files end together
}

// Helper function to compare two text files line-wise or entire content-wise
bool compareTextFiles(const std::string &file1, const std::string &file2)
{
    std::ifstream f1(file1);
    std::ifstream f2(file2);
    if (!f1.is_open() || !f2.is_open())
        return false;

    std::string line1, line2;
    while (true)
    {
        bool b1 = static_cast<bool>(std::getline(f1, line1));
        bool b2 = static_cast<bool>(std::getline(f2, line2));
        if (!b1 && !b2)
            break; // both ended, equal
        if (b1 != b2)
            return false; // one ended earlier
        if (line1 != line2)
            return false;
    }
    return true;
}

// Helper function for simulator
std::vector<std::string> standardCommands(const std::string &filename)
{
    return {
        "load " + filename,
        "break 1",
        "step",
        "regs",
        "show-code",
        "show-stack",
        "run",
        "exit"};
}

// helper function for command generation for cache

std::vector<std::string> standardCacheCommands(const std::string &configPath, const std::string &asmPath)
{
    return {
        "cache_sim enable " + configPath,
        "load " + asmPath,
        "cache_sim status",
        "step",
        "run",
        "cache_sim caches",
        "cache_sim dump cache_dump",
        "cache_sim disable",
        "cache_sim stats",
        "exit"};
}

// helper function for command generation for pipeline

std::vector<std::string> standardPipelineCommands(const std::string &configPath, const std::string &asmPath)
{
    return {
        "load " + asmPath,
        "pipeline enable ",
        "pipeline status",
        "pipeline analyze",
        "pipeline hazards",
        "pipeline stalls",
       // "pipeline stages",
        "pipeline dump "+ configPath +"./pipeline_dump",
        "pipeline forwarding disable",
        "pipeline status ",
        "pipeline analyze",        
        "pipeline hazards",
        "pipeline stalls",
       // "pipeline stages",
        "pipeline clear",
        "pipeline hazards",
        //"step",
        //"pipeline analyze",
        //"pipeline hazards",
        "exit"};
}

int TestRunner::runIntegrationTestSuite(const std::string &testDir, ISAMode mode)
{
    Assembler assembler(std::cin, std::cout, mode);
    DeAssembler deassembler(std::cin, std::cout, mode);
    Simulator simulator(std::cin, std::cout, mode);

    // Create subdirectories
    fs::create_directories(testDir + "/assembler");
    fs::create_directories(testDir + "/deassembler");
    fs::create_directories(testDir + "/simulator");

    // Counters for tests
    int assemblerRuns = 0, assemblerPasses = 0;
    int deassemblerRuns = 0, deassemblerPasses = 0;
    int simulatorRuns = 0, simulatorPasses = 0;

    for (const auto &entry : fs::directory_iterator(testDir))
    {
        std::string path = entry.path().string();
        std::string stem = entry.path().stem().string();
        std::string ext = entry.path().extension().string();

        if (ext == ".s" || ext == ".asm")
        {
            // Read .s file lines
            std::ifstream asmFile(path);
            if (!asmFile.is_open())
            {
                std::cerr << "[ERROR] Cannot open " << path << "\n";
                continue;
            }
            std::vector<std::string> asmLines;
            std::string line;
            assemblerRuns++;
            while (std::getline(asmFile, line))
            {
                asmLines.push_back(line);
            }

            // Assemble
            std::vector<uint32_t> machineCodes = assembler.assemble(asmLines);

            // Write assembled binary to assembler/<stem>.b
            std::string binPath = testDir + "/assembler/" + stem + ".b";
            std::ofstream binFile(binPath, std::ios::binary);
            for (uint32_t code : machineCodes)
            {
                binFile << std::hex << std::setw(8) << std::setfill('0') << code << "\n";
            }

            binFile.close();

            // Compare assembler output .b with expected assembler/<stem>.expect (binary)
            std::string expectedBinPath = testDir + "/assembler/" + stem + ".expect";
            // bool assemblerPass = compareBinaryFiles(binPath, expectedBinPath);
            bool assemblerPass = compareTextFilesNormalized(binPath, expectedBinPath);
            if (assemblerPass)
            {
                assemblerPasses++;
                std::cout << "[PASS] Assembler Test: " << stem << ".s\n";
            }
            else
            {
                std::cout << "[FAIL] Assembler Test: " << stem << ".s (mismatch with " << expectedBinPath << ")\n";
            }

            // Run simulator on assembled code
            // 1. Generate standard test commands
            std::vector<std::string> commands = standardCommands(path);
            // 2. Convert to input stream
            std::stringstream commandInput;
            for (const auto &cmd : commands)
            {
                commandInput << cmd << "\n";
            }
            simulatorRuns++;
            std::ostringstream simOutput;
            InteractiveSimulator interactiveSim(commandInput, simOutput, mode);
            interactiveSim.runInteractive();

            // Write simulator output to simulator/<stem>.out
            std::string simOutPath = testDir + "/simulator/" + stem + ".out";
            std::ofstream simOutFile(simOutPath);
            simOutFile << simOutput.str();
            simOutFile.close();

            // Compare simulator output with testDir/<stem>.sim.expect (text)
            std::string simExpectPath = testDir + "/simulator/" + stem + ".expect";
            bool simPass = compareTextFiles(simOutPath, simExpectPath);
            if (simPass)
            {
                simulatorPasses++;
                std::cout << "[PASS] Simulator Test: " << stem << ".s\n";
            }
            else
            {
                std::cout << "[FAIL] Simulator Test: " << stem << ".s (mismatch with " << simExpectPath << ")\n";
            }
        }
        else if (ext == ".b")
        {

            deassemblerRuns++;
            std::ifstream binFile(path); // No std::ios::binary
            std::vector<uint32_t> machineCodes;
            std::string line;
            while (std::getline(binFile, line))
            {
                uint32_t code = std::stoul(line, nullptr, 16);
                machineCodes.push_back(code);
            }
            binFile.close();

            // Deassemble to lines of assembly
            std::vector<std::string> disassembled = deassembler.disassemble(machineCodes);

            // Write disassembled output to deassembler/<stem>.from_b.s
            std::string outPath = testDir + "/deassembler/" + stem + ".s";
            std::ofstream outFile(outPath);
            for (const auto &l : disassembled)
            {
                outFile << l << "\n";
            }
            outFile.close();

            // Compare disassembled output text with deassembler/<stem>.expect
            std::string expectPath = testDir + "/deassembler/" + stem + ".expect";
            bool deassemblerPass = compareTextFilesIgnoreWhitespace(outPath, expectPath);
            if (deassemblerPass)
            {
                deassemblerPasses++;
                std::cout << "[PASS] Deassembler Test: " << stem << ".b\n";
            }
            else
            {
                std::cout << "[FAIL] Deassembler Test: " << stem << ".b (mismatch with " << expectPath << ")\n";
            }
        }
    }

    // Print summary
    std::cout << "Integration Test Summary:\n";
    std::cout << "Assembler Tests run: " << assemblerRuns << ", passed: " << assemblerPasses << "\n";
    std::cout << "Deassembler Tests run: " << deassemblerRuns << ", passed: " << deassemblerPasses << "\n";
    std::cout << "Simulator Tests run: " << simulatorRuns << ", passed: " << simulatorPasses << "\n";

    return true;
}

bool TestRunner::runTest(const std::string &title, const std::vector<std::string> &asmLines, ISAMode mode)
{

    Assembler assembler(std::cin, std::cout, mode);
    DeAssembler deassembler(std::cin, std::cout, mode);
    std::vector<uint32_t> machineCodes = assembler.assemble(asmLines);
    // labels wont work in a round trip fashion without these
    deassembler.setLabelSet(assembler.getLabelSet());
    std::vector<std::string> disassembled = deassembler.disassemble(machineCodes);

    std::vector<std::string> filteredDisassembled = disassembled;
    bool allPass = true;
    if (asmLines.size() != filteredDisassembled.size())
    {
        std::cout << "Size mismatch: asmLines has " << asmLines.size() << " lines, disassembled has " << filteredDisassembled.size() << " lines.\n";
        allPass = false;
        for (const auto &line : asmLines)
        {
            std::cout << line << "\n";
        }

        std::cout << "\nMachine Codes:\n";
        for (const auto &code : machineCodes)
        {
            std::cout << "0x" << std::hex << std::setw(8) << std::setfill('0') << code << "\n";
        }

        std::cout << "\nDisassembled Output:\n";
        for (const auto &line : disassembled)
        {
            std::cout << line << "\n";
        }
        return allPass;
    }

    size_t minSize = std::min(asmLines.size(), filteredDisassembled.size());
    for (size_t i = 0; i < minSize; ++i)
    {
        if (asmLines[i] != filteredDisassembled[i])
        {
            std::cout << "Row " << i << ": fail\n";
            std::cout << "Expected: " << asmLines[i] << "\n";
            std::cout << "Got:      " << filteredDisassembled[i] << "\n";
            allPass = false;
        }
    }
    if (allPass)
    {
        std::cout << "[PASS] " << title << " \n";
        return allPass;
    }
    return allPass;
}

int TestRunner::unitTests()
{
    int totalTests = 0, passedTests = 0;
    // R-format test
    if (runTest("R-format Instructions", {"add x3, x4, x7",
                                          "sub x1, x2, x3",
                                          "and x8, x9, x10",
                                          "or x11, x12, x13",
                                          "xor x14, x15, x16",
                                          "sll x17, x18, x19",
                                          "srl x20, x21, x22",
                                          "sra x23, x24, x25"}))
        passedTests++;
    totalTests++;
    ;

    // I-format test
    if (runTest("I-format Instructions", {"addi x1, x2, 10",
                                          "andi x3, x4, 15",
                                          "ori x5, x6, 20",
                                          "xori x7, x8, 25",
                                          "slli x9, x10, 2",
                                          "srli x11, x12, 3",
                                          "srai x13, x14, 1",
                                          "lhu x19, 4(x20)",
                                          "lbu x21, 0(x22)"}))
        passedTests++;
    ;
    totalTests++;
    ;

    // S-format test
    if (runTest("S-format Instructions", {"sw x3, 12(x4)",
                                          "sh x5, 4(x6)",
                                          "sb x7, 0(x8)"}))
        passedTests++;
    ;
    ;
    totalTests++;
    ;

    // B-format test
    if (runTest("B-format Instructions", {"beq x1, x2, 8",
                                          "bne x3, x4, 12",
                                          "blt x5, x6, 16",
                                          "bge x7, x8, 20",
                                          "bltu x9, x10, 24",
                                          "bgeu x11, x12, 28"}))
        passedTests++;
    ;
    ;
    ;
    totalTests++;
    ;

    // U/J-format test
    if (runTest("U/J-format Instructions", {"lui x1, 0x10000",
                                            "jal x2, 16",
                                            "jalr x3, 0(x4)"}))
        passedTests++;
    ;
    totalTests++;
    ;

    // RV32IM tests
    if (runTest("RV32IM Instructions", {
                                           "mul x1, x2, x3",      // Basic multiply
                                           "mulh x4, x5, x6",     // High multiply signed
                                           "mulhsu x7, x8, x9",   // High multiply signed-unsigned
                                           "mulhu x10, x11, x12", // High multiply unsigned
                                           "div x1, x2, x3",      // Signed division
                                           "divu x4, x5, x6",     // Unsigned division
                                           "rem x7, x8, x9",      // Signed remainder
                                           "remu x10, x11, x12"   // Unsigned remainder
                                       },
                ISAMode::RV32IM))
        passedTests++;
    ;
    totalTests++;
    ;

    // ========== RV64I Extension Tests ==========
    if (runTest("RV64I Extension Tests", {
                                             "addw x1, x2, x3",    // Add word
                                             "subw x4, x5, x6",    // Subtract word
                                             "sllw x7, x8, x9",    // Shift left logical word
                                             "srlw x10, x11, x12", // Shift right logical word
                                             "sraw x13, x14, x15", // Shift right arithmetic word
                                             "addiw x1, x2, 100",  // Add immediate word
                                             "slliw x3, x4, 5",    // Shift left logical immediate word
                                             "srliw x5, x6, 3",    // Shift right logical immediate word
                                             "sraiw x7, x8, 2",    // Shift right arithmetic immediate word
                                             "ld x1, 0(x2)",       // Load doubleword
                                             "sd x3, 8(x4)",       // Store doubleword
                                             "lwu x5, 4(x6)"       // Load word unsigned
                                         },
                ISAMode::RV64I))
        passedTests++;
    ;
    totalTests++;
    ;

    // ========== RV64I Extension Tests ==========
    if (runTest("RV64IM Extension Tests", {
                                              "mulw x1, x2, x3",    // Multiply word
                                              "divw x4, x5, x6",    // Divide word signed
                                              "divuw x7, x8, x9",   // Divide word unsigned
                                              "remw x10, x11, x12", // Remainder word signed
                                              "remuw x13, x14, x15" // Remainder word unsigned
                                          },
                ISAMode::RV64IM))
        passedTests++;
    ;
    totalTests++;
    ;

    std::cout << "Unit Test Summary: " << passedTests << "/" << totalTests << " passed.\n";
    return passedTests == totalTests ? 0 : 1;
}

int TestRunner::integrationTests()
{
    int totalTests = 0, passedTests = 0;

    // Integrated test: GCD, factorial, addition/subtraction of 10 numbers
    if (runTest("Integrated Test: GCD, Factorial, Add/Sub 10 Numbers", {
                                                                           // GCD Loop (mocked)
                                                                           "beq x5, x6, 8",    // if x5 == x6 skip
                                                                           "blt x5, x6, 4",    // if x5 < x6
                                                                           "sub x5, x5, x6",   // x5 -= x6
                                                                           "jal x0, -12",      // loop back
                                                                           "sub x6, x6, x5",   // else x6 -= x5
                                                                           "jal x0, -16",      // loop back
                                                                                               // Factorial loop (mocked incorrectly)
                                                                           "addi x1, x0, 5",   // x1 = 5
                                                                           "addi x2, x0, 1",   // x2 = 1 (result)
                                                                           "beq x1, x0, 8",    // done?
                                                                           "addi x1, x1, -1",  // x1--
                                                                           "jal x0, -12",      // loop back
                                                                                               // Add 10 numbers
                                                                           "addi x3, x0, 0",   // sum = 0
                                                                           "addi x4, x0, 1",   // start val
                                                                           "addi x5, x0, 10",  // count = 10
                                                                           "add x3, x3, x4",   // sum += val
                                                                           "addi x4, x4, 1",   // val++
                                                                           "addi x5, x5, -1",  // count--
                                                                           "bne x5, x0, -12",  // loop
                                                                                               // Subtract 10 numbers
                                                                           "addi x6, x0, 100", // start = 100
                                                                           "addi x7, x0, 10",  // count = 10
                                                                           "sub x6, x6, x7",   // subtract
                                                                           "addi x7, x7, -1",  // count--
                                                                           "bne x7, x0, -8"    // loop
                                                                       }))
        passedTests++;
    totalTests++;

    // Assignment provided tests
    if (runTest("Assignment Instructions", {"add x3, x4, x7",
                                            "beq x4, x7, L1",
                                            "add x1, x2, x3",
                                            "L1:",
                                            "addi x5, x6, 12",
                                            "jal x0, L2",
                                            "sb x5, 12(x6)",
                                            "lui x9, 0x10000",
                                            "L2:",
                                            "addi x9, x10, 12"}))
        passedTests++;
    totalTests++;
    ;

    if (runTest("Label Instructions", {"beq x4, x7, L1",
                                       "add x1, x2, x3",
                                       "L1:",
                                       "addi x5, x6, 12"}))
        passedTests++;
    totalTests++;
    ;

    // Integration test: Print integer
    if (runTest("Print integer", {"addi x10, x0, -10",
                                  "addi x11, x0, 10",
                                  "addi x12, x0, 1",
                                  //"jal loopPrint",
                                  "jal x0, printNewLine",
                                  "printNewLine:",
                                  // "loopPrint:",
                                  //  "la a0, newline",
                                  "jalr x0, 0(x1)"}))
        passedTests++;
    totalTests++;

    std::cout << "Integration Test Summary: " << passedTests << "/" << totalTests << " passed.\n";
    return passedTests == totalTests ? 0 : 1;
}

/**
 * @brief Runs cache simulator integration tests located in the specified root directory.
 *
 * This function recursively searches for test directories under the given root directory (default: "Test/CacheSimulator").
 * Each test directory is expected to contain the following files:
 *   - config: Cache configuration file.
 *   - test.asm: Assembly file to be loaded and executed.
 *   - test.expected: Expected output file for the test.
 *
 * For each valid test directory:
 *   1. Constructs a sequence of simulator commands to enable cache simulation, load the assembly file, run the program,
 *      display cache contents and statistics, and exit.
 *   2. Writes these commands to a temporary input file.
 *   3. Executes the simulator (riscv_sim) with input redirected from the temporary file and output redirected to "test.out".
 *   4. Compares the simulator output ("test.out") with the expected output ("test.expect"), ignoring line ending differences
 *      and trailing spaces.
 *   5. Reports pass/fail status for each test and counts the number of tests run and passed.
 *   6. Cleans up temporary files after each test.
 *
 * At the end, prints a summary of the number of cache tests run and passed.
 *
 * @param rootDir The root directory containing cache simulator test cases (default: "Test/CacheSimulator").
 * @return 0 if all tests pass, 1 otherwise.
 */

int TestRunner::runCacheTests(const std::string &rootDir)
{
    int testsRun = 0;
    int testsPassed = 0;

    for (auto &entry : fs::recursive_directory_iterator(rootDir))
    {
        if (!entry.is_directory())
            continue;

        fs::path dirPath = entry.path();
        fs::path configFile = dirPath / "config";
        fs::path asmFile = dirPath / "test.asm";
        fs::path expectedFile = dirPath / "test.expect";
        fs::path outFile = dirPath / "test.out";

        if (!fs::exists(configFile) || !fs::exists(asmFile) || !fs::exists(expectedFile))
        {
            continue;
        }

        // Generate standard commands
        std::vector<std::string> commands = standardCacheCommands(configFile.string(), asmFile.string());

        // Convert to input stream
        std::stringstream commandInput;
        for (const auto &cmd : commands)
        {
            commandInput << cmd << "\n";
        }

        // Run simulator
        std::ostringstream simOutput;
        InteractiveSimulator interactiveSim(commandInput, simOutput);
        // Set a fixed random number seed so that tests dont give different results everytime and fail
        interactiveSim.getCacheSimulator().setRandomSeed(12345);
        interactiveSim.runInteractive();
        // 💡 Force log flush
        interactiveSim.invalidateCache(); // flush the output of log generator
        // Append .log file if it exists in current working directory
        fs::path logFile = fs::current_path() / (asmFile.stem().string() + ".log");
        // std::cout << "Looking for log at: " << logFile << "\n";

        if (fs::exists(logFile))
        {
            std::ifstream logIn(logFile);
            if (logIn)
            {
                simOutput << "\n=== Log File ===\n";
                simOutput << logIn.rdbuf(); // append entire log content
            }
        }

        // Append cache dump file if it exists
        fs::path cacheDumpFile = fs::current_path() / "cache_dump";
        if (fs::exists(cacheDumpFile))
        {
            std::ifstream dumpIn(cacheDumpFile);
            if (dumpIn)
            {
                simOutput << "\n=== Cache Dump ===\n";
                simOutput << dumpIn.rdbuf(); // append cache dump content
            }
        }

        // Write output to test.out
        std::ofstream out(outFile);
        out << simOutput.str();
        out.flush(); // Ensure file content is written before reading it again

        // Compare with expected output
        bool pass = compareTextFilesNormalized(outFile.string(), expectedFile.string());
        if (pass)
        {
            std::cout << "[PASS] Cache Test: " << asmFile << "\n";
            ++testsPassed;
        }
        else
        {
            std::cout << "[FAIL] Cache Test: " << asmFile << " (output mismatch)\n";
        }
        ++testsRun;
    }

    std::cout << "Cache tests run: " << testsRun << ", passed: " << testsPassed << "\n";
    return testsRun == testsPassed ? 0 : 1;
}

/**
 * @brief Runs hazard detector tests located in the specified directory.
 *
 * This function tests the hazard detector component by:
 *   1. Loading assembly files and running pipeline simulation
 *   2. Using pipeline commands to enable hazard detection
 *   3. Comparing detected hazards with expected results
 *   4. Testing various pipeline analysis features
 *
 * Each test directory should contain:
 *   - test.asm: Assembly file to analyze
 *   - test.expect: Expected hazard detection output
 *   - test.commands: Custom pipeline commands (optional)
 *
 * @param rootDir The root directory containing hazard detector test cases
 * @return 0 if all tests pass, 1 otherwise
 */
int TestRunner::runHazardTests(const std::string &rootDir, ISAMode mode)
{
    int testsRun = 0;
    int testsPassed = 0;

    for (auto &entry : fs::recursive_directory_iterator(rootDir))
    {
        if (!entry.is_directory())
            continue;

        fs::path dirPath = entry.path();
        fs::path asmFile = dirPath / "test.asm";
        fs::path expectedFile = dirPath / "test.expect";
        fs::path commandsFile = dirPath / "test.commands";
        fs::path outFile = dirPath / "test.out";
        fs::path dumpFile = dirPath / "pipeline_dump";
        fs::path expectedDumpFile = dirPath / "pipeline_dump.expect";

        if (!fs::exists(asmFile) || !fs::exists(expectedFile))
        {
            continue;
        }

       // std::cout << "Running hazard test: " << dirPath.filename() << "\n";

        // Generate pipeline commands for hazard detection
        std::vector<std::string> commands= standardPipelineCommands(dirPath.string(), asmFile.string());
        

        // Convert commands to input stream
        std::stringstream commandInput;
        for (const auto &cmd : commands)
        {
            commandInput << cmd << "\n";
        }

        // Run simulator with pipeline commands
        std::ostringstream simOutput;
        InteractiveSimulator interactiveSim(commandInput, simOutput,mode);
        interactiveSim.runInteractive();

        // Write output
        std::ofstream out(outFile);
        out << simOutput.str();
        out.close();

        // Compare with expected output
        bool pass1 = compareTextFilesNormalized(outFile.string(), expectedFile.string());
        bool pass2 = compareTextFilesNormalized(dumpFile.string(), expectedDumpFile.string());

        if (pass1 && pass2)
        {
            std::cout << "[PASS] Hazard Detection: " << asmFile << "\n";
            ++testsPassed;
        }
        else
        {
            std::cout << "[FAIL] Hazard Detection: " << asmFile << " (output mismatch)\n";
        }
        ++testsRun;
    }

    std::cout << "Hazard tests run: " << testsRun << ", passed: " << testsPassed << "\n";
    return testsRun == testsPassed ? 0 : 1;
}

// Run all tests
int TestRunner::runAllTests(ISAMode mode)
{

    unitTests();
    integrationTests();
    runIntegrationTestSuite("Test", mode);
    runCacheTests();
    runHazardTests("Test/PipelineHazards",mode);
    return 0;

}