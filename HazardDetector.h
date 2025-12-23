
#ifndef HAZARDDETECTOR_H
#define HAZARDDETECTOR_H


#include <vector>
#include <iostream>
#include "stdlib.h"
#include "InstructionInstance.h"




enum PipelineType {
        PIPELINE_3_STAGE,   // Fetch, Decode/Execute, Writeback
        PIPELINE_5_STAGE,   // Fetch, Decode, Execute, Memory, Writeback
        PIPELINE_7_STAGE    // Fetch, Decode, Rename, Issue, Execute, Memory, Writeback
    };

struct Hazard {
        enum Type { RAW, WAW, WAR, CONTROL, STRUCTURAL, CACHE_MISS };
        Type type;
        const InstructionInstance* instruction1;
        const InstructionInstance* instruction2; 
        int registerNum =  -1;
        int stallCycles =0; // How many cycles this hazard would cause
        uint32_t memoryAddress =0 ; // For cache-related hazards
        std::string description;
        std::string solution;
        friend std::ostream& operator<<(std::ostream& os, const Hazard& hazard) ;
    };


// statistics
struct PipelineStats {
    int totalStalls = 0;
    int rawStalls = 0;
    int wawStalls = 0;
    int warStalls = 0;
    int controlStalls = 0;
    int structuralStalls = 0;
    int cacheStalls = 0;
    int totalInstructions = 0;
    int totalCycles = 0;
    
    void reset() {
        totalStalls = rawStalls = wawStalls = warStalls = 0;
        controlStalls = structuralStalls = cacheStalls = 0;
        totalInstructions = totalCycles = 0;
    }
};

class HazardDetector {
    private:
        std::vector<InstructionInstance> instructionWindow;
        int windowSize = 6; // Look ahead window
        PipelineType pipelineType = PIPELINE_5_STAGE; // Default pipeline type
        bool cacheEnabled = false;
        
        // Cache parameters for stall calculation
        int cacheHitLatency = 1;
        int cacheMissPenalty = 10; // Typical L1 miss penalty

        //Pipeline simulation state
        bool pipelineEnabled = false;
        bool forwardingEnabled = true;
        PipelineStats stats;
        int currentCycle = 0;
        std::vector<Hazard> detectedHazards;
            
    public:
        HazardDetector(std::istream& in = std::cin, std::ostream& out = std::cout)
            : in(in), out(out) {
        }
        void enablePipeline();
        void disablePipeline();
        void showPipelineStatus();
        void showHazards();
        void showStallStats() ;
    
        void enableForwarding() ;
        void disableForwarding();
        void clearPipeline();
        void dumpPipelineState(const std::string& filename);
        bool isPipelineEnabled() const {
            return pipelineEnabled;
        }   

        std::vector<Hazard> detectHazards(const std::vector<InstructionInstance>& program, int currentPC, 
                                      bool cacheEnabled = false);
        void setPipelineType(PipelineType type);
        void performanceAnalysis(std::vector<InstructionInstance> &program, bool cacheEnabled);
        void analyzeHazards(std::vector<InstructionInstance> &program,int pc, bool cacheEnabled);
 
private:
        std::istream &in;
        std::ostream &out;
      
        int calculateRAWStallCycles(const InstructionInstance& inst1, int distance);
        int calculateWAWStallCycles();
        int calculateWARStallCycles() ;
        int calculateControlStallCycles(const InstructionInstance& inst);
        int calculateCacheStallCycles() const;
        int calculateStructuralStallCycles(const InstructionInstance& inst1, const InstructionInstance& inst2, int distance); 

        std::string getSolutionForRAW(int stallCycles);
        std::string getSolutionForWAR(int stallCycles);


        std::string getPipelineTypeName();
        std::string getHazardTypeName(Hazard::Type type);
        Hazard makeRAWHazard(const InstructionInstance* inst1, const InstructionInstance* inst2, int regNum, int stallCycles);
        Hazard makeWAWHazard(const InstructionInstance* inst1, const InstructionInstance* inst2, int regNum, int stallCycles);
        Hazard makeWARHazard(const InstructionInstance* inst1, const InstructionInstance* inst2, int regNum, int stallCycles);
        Hazard makeStructuralHazard(const InstructionInstance* inst1, const InstructionInstance* inst2, const std::string& unitName, int stallCycles);
        Hazard makeControlHazard(const InstructionInstance* inst1, const InstructionInstance* inst2, const std::string& reason, int stallCycles);
        Hazard makeCacheHazard(const InstructionInstance* inst1, const InstructionInstance* inst2, uint32_t address, int stallCycles);
};


#endif // HAZARDDETECTOR_H