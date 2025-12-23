#ifndef PIPELINE_SIMULATOR_H
#define PIPELINE_SIMULATOR_H

#include "Simulator.h"
#include <array>
#include <optional>
#include <iostream>
/*
class PipelineSimulator : public Simulator {
    
public:
    PipelineSimulator(std::istream& in = std::cin, std::ostream& out = std::cout);

    void load(const std::vector<InstructionInstance>& instructions) override;
    void run() override;
    int step() override;
    void reset() override;
    void printPipelineState() const;

protected:
    enum class Stage {
        Fetch = 0,
        Decode,
        Execute,
        Memory,
        WriteBack,
        NUM_STAGES
    };

    struct PipelineRegister {
        std::optional<InstructionInstance> instruction;
        int stallCycles = 0;

        void clear() {
            instruction.reset();
            stallCycles = 0;
        }
    };

    // Pipeline registers for each stage
    std::array<PipelineRegister, static_cast<size_t>(Stage::NUM_STAGES)> pipelineRegisters;

    int pc = 0;  // program counter for fetch

    bool pipelineStalled = false;
    bool programFinished = false;

    // Pipeline stage functions
    void fetch();
    void decode();
    void execute();
    void memoryAccess();
    void writeBack();

    void advancePipeline();
    bool detectHazard();
    void stallPipeline();
    void flushPipeline();
    
    // Helper to convert enum Stage to string for printing
    std::string stageToString(Stage stage) const; 
};
*/
#endif // PIPELINE_SIMULATOR_H
