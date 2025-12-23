
#include "Assembler.h"
#include "DeAssembler.h"
#include "InstructionSet.h"
#include "InteractiveSimulator.h"

class TestRunner
{
    bool runTest(const std::string &title, const std::vector<std::string> &asmLines, ISAMode mode = ISAMode::RV32I);

public:
    int unitTests();
    int integrationTests();
    int runIntegrationTestSuite(const std::string &testDir, ISAMode mode = ISAMode::RV32I);
    int runCacheTests(const std::string &rootDir = "Test/CacheSimulator");
    int runAllTests(ISAMode mode = ISAMode::RV32I);
    int runHazardTests(const std::string &rootDir,ISAMode mode= ISAMode::RV32I);

};
