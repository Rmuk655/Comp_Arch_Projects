#include <iostream>
#include <fstream>
#include <algorithm>
#include <unordered_set>
#include "HazardDetector.h"


    std::ostream& operator<<(std::ostream& os, const Hazard& hazard) {

        if (hazard.instruction2) {
            os << "   Instruction: " << hazard.instruction2->originalCode;
            os << " at PC=0x" << std::hex << hazard.instruction2->pc << std::dec << "\n";
            os << "   Depends on: " << hazard.instruction1->originalCode;
            os << " (PC=0x" << std::hex << hazard.instruction1->pc << std::dec << ")\n";

            int distance = std::abs((int)(hazard.instruction2->pc - hazard.instruction1->pc)) / 4;
            os << "   Distance: " << distance << " instruction" << (distance == 1 ? "" : "s") << "\n";
        } else {
            // Fallback if instruction2 is not set
            os << "   Instruction: " << hazard.instruction1->originalCode;
            os << " at PC=0x" << std::hex << hazard.instruction1->pc << std::dec << "\n";
        }
            
        if ((hazard.type == Hazard::Type::RAW || hazard.type == Hazard::Type::WAW || 
            hazard.type == Hazard::Type::WAR) && hazard.registerNum >= 0) {
                os << "   Register: x" << hazard.registerNum ;
        }

          
            
            os << "   Stall cycles: " << hazard.stallCycles << "\n";
            if (hazard.stallCycles == 0) {
                if(hazard.type == Hazard::Type::WAW)
                    os << "   Note: WAW hazard resolved by instruction separation or writeback timing\n";
                else
                    os << "   Note: Hazard resolved by separation or forwarding\n";

            }
                
            if (hazard.type == Hazard::Type::CACHE_MISS)
                    os << "\n  Memory Address: 0x" << std::hex << hazard.memoryAddress << std::dec;
                      
            os << "   Description: " << hazard.description << "\n";
            os << "   Solution: " << hazard.solution << "\n";
            return os;
                  
    }

    std::string HazardDetector::getSolutionForRAW(int stallCycles) {
        if (stallCycles == 0) {
            return forwardingEnabled ? "No stall needed (forwarding available)" : "No stall needed";
        }
        if (stallCycles == 1) {
            return forwardingEnabled ? "1 cycle stall or forwarding" : "1 cycle stall required";
        }
        return std::to_string(stallCycles) + " cycle stall required";    
    }
    
    std::string HazardDetector::getSolutionForWAR(int stallCycles) {
        if (stallCycles == 0) return "No stall needed (no conflict or handled by reordering)";
        if (stallCycles == 1) return "1 cycle stall or instruction reordering";
        return std::to_string(stallCycles) + " cycle stall — consider reordering or register renaming";
    }

    // Function to create a RAW (Read After Write) hazard
    Hazard HazardDetector::makeRAWHazard(const InstructionInstance* inst1, const InstructionInstance* inst2, int regNum, int stallCycles) {
        Hazard hazard;

        hazard.type = Hazard::Type::RAW;  // Type of hazard
        hazard.instruction1 = inst1;       // First instruction (writes to reg)
        hazard.instruction2 = inst2;       // Second instruction (reads from reg)
        hazard.registerNum = regNum;        // Register number causing the hazard
        hazard.stallCycles = stallCycles;  // Number of stall cycles needed
        hazard.description = "RAW Hazard: Read-after-Write on x" + std::to_string(regNum); // Description
        hazard.solution = getSolutionForRAW(stallCycles); // How to fix it
    
        return hazard;
    }
 
    // Function to create a WAW (Write After Write) hazard
    Hazard HazardDetector::makeWAWHazard(const InstructionInstance* inst1, const InstructionInstance* inst2, int regNum, int stallCycles) {
        Hazard hazard;

        hazard.type = Hazard::Type::WAW;
        hazard.instruction1 = inst1;
        hazard.instruction2 = inst2;
        hazard.registerNum = regNum;
        hazard.stallCycles = stallCycles;
        
        hazard.description = "WAW Hazard: Write-after-Write on x" + std::to_string(regNum);
        hazard.solution = "Reorder writes or stall to avoid overwriting";

        return hazard;
    }

    // Function to create a WAR (Write After Read) hazard
    Hazard HazardDetector::makeWARHazard(const InstructionInstance* inst1, const InstructionInstance* inst2, int regNum, int stallCycles) {
        Hazard hazard;

        hazard.type = Hazard::Type::WAR;
        hazard.instruction1 = inst1;
        hazard.instruction2 = inst2;
        hazard.registerNum = regNum;
        hazard.stallCycles = stallCycles;
        hazard.description = "WAR Hazard: Write-after-Read on x" + std::to_string(regNum);
        hazard.solution = getSolutionForWAR(stallCycles);

        return hazard;
    }

    Hazard HazardDetector::makeStructuralHazard(const InstructionInstance* inst1, const InstructionInstance* inst2, const std::string& unitName, int stallCycles) {
        Hazard hazard;
        hazard.type = Hazard::Type::STRUCTURAL;
        hazard.instruction1 = inst1;
        hazard.instruction2 = inst2;
        hazard.stallCycles = stallCycles;
        hazard.description = "Structural hazard: both instructions use " + unitName;
        hazard.solution = "Stall pipeline until " + unitName + " is available";
        return hazard;
    }

    Hazard HazardDetector::makeControlHazard(const InstructionInstance* inst1, const InstructionInstance* inst2, const std::string& reason, int stallCycles) {
        Hazard hazard;
        hazard.type = Hazard::Type::CONTROL;
        hazard.instruction1 = inst1;
        hazard.instruction2 = inst2;
        hazard.stallCycles = stallCycles;
        hazard.description = "Control hazard from " + reason ;
        hazard.solution = "Use branch prediction or delayed branching (" + std::to_string(stallCycles) + " cycles)";
        return hazard;
    }

    Hazard HazardDetector::makeCacheHazard(const InstructionInstance* inst1, const InstructionInstance* inst2, uint32_t address, int stallCycles) {
        Hazard hazard;
        hazard.type = Hazard::Type::CACHE_MISS;
        hazard.instruction1 = inst1;
        hazard.instruction2 = inst2;
        hazard.memoryAddress = address;
        hazard.stallCycles = stallCycles;
        hazard.description = "Cache miss at address 0x" + 
            std::to_string(address); // or use hex formatting if needed
        hazard.solution = "Wait for memory access or prefetch cache lines";
        return hazard;
    }

    std::vector<Hazard> HazardDetector::detectHazards( const std::vector<InstructionInstance>& program, int currentPC, bool cacheEnabled) {

        std::vector<Hazard> hazards;
        //int windowSize = 6;

        for (int i = currentPC; i < (int)program.size(); ++i) {
            const InstructionInstance& inst1 = program[i];
            std::string name1 = inst1.instruction->mnemonic;

            // ----- Control Hazards -----
            if (inst1.instruction->isBranchInstruction(name1) || inst1.instruction->isJumpInstruction(name1)) {
                int stall = calculateControlStallCycles(inst1);
                std::string reason ;
                if(name1 == "jalr") {
                    reason = "Function return: "; 
                }
                else reason = "Branch instruction: " ;
                hazards.push_back(makeControlHazard(&inst1, nullptr, reason + name1, stall));        
                stats.controlStalls += stall;
                stats.totalStalls += stall;
            }

            // ----- Cache Hazards (static estimation) -----
            if (cacheEnabled && (inst1.instruction->isLoadInstruction(name1) || inst1.instruction->isStoreInstruction(name1))) {
                uint32_t dummyAddress = 0;  // Or any heuristic estimation of address
                int stall = calculateCacheStallCycles(); 
                Hazard h = makeCacheHazard(&inst1, nullptr, dummyAddress,stall);
                hazards.push_back(h);
                stats.cacheStalls += h.stallCycles;
                stats.totalStalls += h.stallCycles;
            }
    
            for (int j = i + 1; j < (int)program.size(); ++j) {
                const InstructionInstance& inst2 = program[j];

                int distance = j - i;

                // RAW (Read After Write): inst2 reads rs1 or rs2 that inst1 writes to rd
                if (inst1.rd != -1 && inst1.rd != 0) {
                    bool hazardDetected = false;
                    
                    if (inst2.rs1 != -1 && inst2.rs1 == inst1.rd) {
                        hazardDetected = true;
                    }
                    if (inst2.rs2 != -1 && inst2.rs2 == inst1.rd) {
                        hazardDetected = true;
                    }

                    if (hazardDetected) {
                        int stall = calculateRAWStallCycles(inst1, distance);
                        Hazard h = makeRAWHazard(&inst1, &inst2, inst1.rd, stall);
                        hazards.push_back(h);
                        stats.rawStalls += h.stallCycles;
                        stats.totalStalls += h.stallCycles;
                        
                    }
                }
                // WAW (Write After Write): both inst1 and inst2 write to same rd
                if (inst1.rd != -1 && inst2.rd != -1 && inst1.rd == inst2.rd && inst1.rd != 0) {
                    int stall = calculateWAWStallCycles();
                    Hazard h = makeWAWHazard(&inst1, &inst2, inst1.rd, stall);
                    hazards.push_back(h);
                    stats.wawStalls += h.stallCycles;
                    stats.totalStalls += h.stallCycles;
                    
                }

                // WAR (Write After Read): inst1 reads rs1 or rs2 that inst2 writes to rd
                if (inst2.rd != -1 && inst2.rd != 0) {
                    bool hazardDetected = false;

                    if (inst1.rs1 != -1 && inst1.rs1 == inst2.rd) {
                        hazardDetected = true;
                    }
                    if (inst1.rs2 != -1 && inst1.rs2 == inst2.rd) {
                        hazardDetected = true;
                    }

                    if (hazardDetected) {
                        int stall = calculateWARStallCycles();
                        Hazard h = makeWARHazard(&inst1, &inst2, inst2.rd, stall);
                        hazards.push_back(h); 
                        stats.warStalls += h.stallCycles;
                        stats.totalStalls += h.stallCycles;                        
                    }
                }
               // Structural Hazards: check if both instructions use the same resource
                int structStall = calculateStructuralStallCycles(inst1, inst2, distance);
                    if (structStall > 0) {
                        std::string unit = "Functional unit";  
                        if (inst1.instruction->isMulDivInstruction(name1)) unit = "Mul/Div Unit";
                        else if (inst1.instruction->isALUInstruction(name1)) unit = "ALU";
                        else if (inst1.instruction->isLoadInstruction(name1) || inst1.instruction->isStoreInstruction(name1)) unit = "Memory Access Unit";
                        hazards.push_back(makeStructuralHazard(&inst1, &inst2, unit, structStall));
                        stats.structuralStalls += structStall;
                        stats.totalStalls += structStall;
                    }

            }
        }

        return hazards;
    }


    int HazardDetector::calculateRAWStallCycles(const InstructionInstance& inst1,  int distance) {
       const std::string& name = inst1.instruction->mnemonic;
       bool isLoad = inst1.instruction->isLoadInstruction(name);
        
        int baseStallCycles = 0;
        
        switch (pipelineType) {
            case PIPELINE_3_STAGE:
                baseStallCycles = (distance == 1) ? 1 : 0; // Only adjacent instructions cause stalls
                break;
                
            case PIPELINE_5_STAGE:
                if (forwardingEnabled) {
                        if (isLoad) {
                            baseStallCycles = (distance == 1) ? 1 : std::max(0, 2 - distance);
                        } else {
                            baseStallCycles = std::max(0, 1 - distance);  // ALU forwarding from EX/MEM
                        }
                    } else {
                        if (isLoad && distance == 1)
                            baseStallCycles = 2;
                        else if (distance <= 2)
                            baseStallCycles = std::max(0, 3 - distance);
                    }
                    break;                
            case PIPELINE_7_STAGE:
                if (forwardingEnabled) {
                    if (isLoad)
                        baseStallCycles = std::max(0, 2 - distance);
                    else
                        baseStallCycles = std::max(0, 3 - distance);
                } else {
                    if (isLoad && distance <= 2)
                        baseStallCycles = std::max(0, 3 - distance);
                    else if (distance <= 3)
                        baseStallCycles = std::max(0, 4 - distance);
                }
                break;
            default:
                baseStallCycles = 0;
        }
        

        return baseStallCycles;
    }
 
    // In out-of-order pipelines, this could be more complex
    // but for simplicity we assume no WAW or WAR stalls in this implementation
 
    int HazardDetector::calculateWAWStallCycles() {
        return 0; // No stalls needed in in-order pipelines
    }

    int HazardDetector::calculateWARStallCycles() {
        return 0; // No WAR stalls in in-order pipelines
    }

   int HazardDetector::calculateControlStallCycles(const InstructionInstance& inst) {
        // Control hazards depend on branch prediction and pipeline depth
        const std::string& mnemonic = inst.instruction->mnemonic;
            if (!inst.instruction->isBranchInstruction(mnemonic) && !inst.instruction->isJumpInstruction(mnemonic)) {
                return 0;
            }


        
        // Exempt only truly unconditional jumps (like jal)
        if (mnemonic == "jal") {
            return 0;
        }    

        // For jalr, treat as unpredictable jump → control hazard
        if (mnemonic == "jalr") {
            switch (pipelineType) {
                case PIPELINE_3_STAGE: return 1;
                case PIPELINE_5_STAGE: return 2;
                case PIPELINE_7_STAGE: return 3;
                default: return 2;
            }
        }

        // Handle conditional branches
        if (inst.instruction->isBranchInstruction(mnemonic)) {
            
            bool isLoop = inst.imm < 0; //inst.targetPC < inst.pc;  // Backward branch
            
            // Simulate "always taken" prediction for loops
            if (isLoop) {
                // Only mispredict at final iteration
                bool alreadyReported = std::any_of(this->detectedHazards.begin(), this->detectedHazards.end(),[&](const Hazard& h) {
                    return h.type == Hazard::CONTROL && h.instruction1 != nullptr && h.instruction1->pc == inst.pc;});

                if (!alreadyReported) {
                    
                    switch (pipelineType) {
                        case PIPELINE_3_STAGE: return 1;
                        case PIPELINE_5_STAGE: return 2;
                        case PIPELINE_7_STAGE: return 3;
                        default: return 2;
                    }
                } else {
                    return 0; // Already mispredicted once
                }
            } else {
                // Forward branches or unpredictable → always mispredict
                switch (pipelineType) {
                    case PIPELINE_3_STAGE: return 1;
                    case PIPELINE_5_STAGE: return 2;
                    case PIPELINE_7_STAGE: return 3;
                    default: return 2;
                }
            }
        }

        // Default conservative penalty
        return 2;

  }
 
    int HazardDetector::calculateStructuralStallCycles(const InstructionInstance& inst1, const InstructionInstance& inst2, int distance) 
    {
        const std::string& name1 = inst1.instruction->mnemonic;
        const std::string& name2 = inst2.instruction->mnemonic;

        // Structural hazards only occur when the same unit is needed by both instructions
        // at overlapping times (i.e., distance too small for reuse).

        // === 3-stage pipeline: Fetch, Execute, WriteBack ===
        if (pipelineType == PIPELINE_3_STAGE) {
            // ALU unit is used in Execute stage: can't do back-to-back if single ALU
            if (inst1.instruction->isALUInstruction(name1) &&
                inst2.instruction->isALUInstruction(name2) &&
                distance == 0)
                return 1;

            // Load unit used in Execute: also needs 1 cycle gap
            if (inst1.instruction->isLoadInstruction(name1) &&
                inst2.instruction->isLoadInstruction(name2) &&
                distance == 0)
                return 1;
        }

        // === 5/7-stage pipelines: functional units are more specialized ===
        if (pipelineType == PIPELINE_5_STAGE || pipelineType == PIPELINE_7_STAGE) {
            // ALU executes in EX stage — if only one ALU, back-to-back causes stall
            if (inst1.instruction->isALUInstruction(name1) &&
                inst2.instruction->isALUInstruction(name2) &&
                distance == 0)
                return 1;

            // Memory access unit shared by loads and stores (in MEM stage)
            if ((inst1.instruction->isLoadInstruction(name1) || inst1.instruction->isStoreInstruction(name1)) &&
                (inst2.instruction->isLoadInstruction(name2) || inst2.instruction->isStoreInstruction(name2)) &&
                distance < 2)
                return 1;

            // Mult/Div usually uses a long-latency unit — needs 2 cycles spacing
            if (inst1.instruction->isMulDivInstruction(name1) &&
                inst2.instruction->isMulDivInstruction(name2) &&
                distance < 2)
                return 2;
        }

        // No hazard detected
        return 0;
    }

    int HazardDetector::calculateCacheStallCycles() const {
        switch (pipelineType) {
            case PIPELINE_3_STAGE: return 5;  // Assume small penalty due to no separate MEM stage
            case PIPELINE_5_STAGE: return 10; // Reasonable default assuming miss goes to main memory
            case PIPELINE_7_STAGE: return 12; // Slightly higher for deeper pipelines
            default: return 10; // Conservative default
        }
    }


    
     std::string HazardDetector::getHazardTypeName(Hazard::Type type) {
        switch (type) {
            case Hazard::RAW: return "RAW";
            case Hazard::WAW: return "WAW";
            case Hazard::WAR: return "WAR";
            case Hazard::CONTROL: return "Control";
            case Hazard::STRUCTURAL: return "Structural";
            case Hazard::CACHE_MISS: return "Cache Miss";
            default: return "Unknown";
        }
    }

    std::string HazardDetector::getPipelineTypeName() {
        switch (pipelineType) {
            case PIPELINE_3_STAGE: return "3-Stage";
            case PIPELINE_5_STAGE: return "5-Stage";
            case PIPELINE_7_STAGE: return "7-Stage";
            default: return "Unknown";
        }
    }

    
    // 1. pipeline enable
    void HazardDetector::enablePipeline() {
        pipelineEnabled = true;
        stats.reset();
        out << "Pipeline hazard detection enabled\n";
    }
    
    // 2. pipeline disable
    void HazardDetector::disablePipeline() {
        pipelineEnabled = false;
        clearPipeline();
        out << "Pipeline hazard detection disabled\n";
    }
    
    // 3. pipeline status
    void HazardDetector::showPipelineStatus() {
        out << "Pipeline Status:\n";
        out << "  Enabled: " << (pipelineEnabled ? "Yes" : "No") << "\n";
        out << "  Type: " << getPipelineTypeName() << "\n";
        out << "  Forwarding: " << (forwardingEnabled ? "Enabled" : "Disabled") << "\n";
        out << "  Window Size: " << windowSize << "\n";
        out << "  Current Cycle: " << currentCycle << "\n";
        out << "  Cache Simulation: " << (cacheEnabled ? "Enabled" : "Disabled") << "\n";
    }
    
    // 4. pipeline hazards
    void HazardDetector::showHazards() {
        if (detectedHazards.empty()) {
            out << "No hazards detected in current instruction window\n";
            return;
        }

        out << "\n=== Critical Hazards (Stalls > 0) ===\n";
        bool anyCritical = false;
        for (const auto& hazard : detectedHazards) {
            if (hazard.stallCycles > 0) {
                out << hazard << "\n";
                anyCritical = true;
            }
        }
        if (!anyCritical) {
            out << "None\n";
        }

        out << "\n=== Resolved / Non-Critical Hazards ===\n";
        bool anyResolved = false;
        for (const auto& hazard : detectedHazards) {
            if (hazard.stallCycles == 0) {
                out << hazard << "\n";
                anyResolved = true;
            }
        }
        if (!anyResolved) {
            out << "None\n";
        }
    }
   
    // 5. pipeline stalls
    void HazardDetector::showStallStats() {
        out << "Pipeline Stall Statistics:\n";
        out << "  Total Stalls: " << stats.totalStalls << "\n";
        if(stats.rawStalls > 0)
            out << "  RAW Hazards: " << stats.rawStalls << "\n";
        if(stats.wawStalls > 0)
            out << "  WAW Hazards: " << stats.wawStalls << "\n";
        if(stats.warStalls > 0)
            out << "  WAR Hazards: " << stats.warStalls << "\n";
        if(stats.controlStalls > 0)
            out << "  Control Hazards: " << stats.controlStalls << "\n";
        if(stats.structuralStalls > 0)
            out << "  Structural Hazards: " << stats.structuralStalls << "\n";
        if(stats.cacheStalls > 0)
            out << "  Cache Misses: " << stats.cacheStalls << "\n";
        
        out << "  Total Instructions: " << stats.totalInstructions << "\n";
        out << "  Total Cycles: " << stats.totalCycles << "\n";
        if (stats.totalInstructions > 0) {
            out << "  CPI: " << (double)stats.totalCycles / stats.totalInstructions << "\n";
        }
    }
    
    
    // 7. pipeline forwarding enable
    void HazardDetector::enableForwarding() {
        forwardingEnabled = true;
        out << "Data forwarding enabled\n";
    }
    
    // 8. pipeline forwarding disable
    void HazardDetector::disableForwarding() {
        forwardingEnabled = false;
        out << "Data forwarding disabled\n";
    }
    
    // 9. pipeline clear
    void HazardDetector::clearPipeline() {
        currentCycle = 0;
        detectedHazards.clear();
        stats.reset();
          out << "Pipeline cleared\n";
    }
    
    // 10. pipeline dump <filename>
    void HazardDetector::dumpPipelineState(const std::string& filename) {
        std::ofstream file(filename);
        if (!file.is_open()) {
            out << "Error: Cannot open file " << filename << "\n";
            return;
        }
  
        file << "Pipeline Configuration:\n";
        file << "  Type: " << getPipelineTypeName() << "\n";
        file << "  Forwarding: " << (forwardingEnabled ? "Enabled" : "Disabled") << "\n";
        file << "  Window Size: " << windowSize << "\n\n";
        
        file << "\nDetected Hazards:\n";
        for (const auto& hazard : detectedHazards) {
            file << hazard << "\n";
        }
        
        file << "\nStatistics:\n";
        file << "  Total Stalls: " << stats.totalStalls << "\n";
        file << "  Total Instructions: " << stats.totalInstructions << "\n";
        file << "  Total Cycles: " << stats.totalCycles << "\n";
        if (stats.totalInstructions > 0) {
            file << "  CPI: " << (double)stats.totalCycles / stats.totalInstructions << "\n";
        }
        
        file.close();
        out << "Pipeline state dumped to " << filename << "\n";
    }



void HazardDetector::performanceAnalysis(std::vector<InstructionInstance> &program, bool cacheEnabled) {
        
        out << "\n=== PERFORMANCE ANALYSIS ===\n";
        
        // Analyze entire program for hazards
        stats.reset();
        detectedHazards.clear();
        
        detectedHazards = detectHazards(program, 0, cacheEnabled);
        
        
        out << "Program instructions: " << program.size() << "\n";
        out << "Total hazards detected: " << (int) detectedHazards.size() << "\n";
        out << "Total stall cycles: " << stats.totalStalls << "\n";
        stats.totalInstructions = program.size();
        stats.totalCycles = stats.totalInstructions + stats.totalStalls;

        if(stats.totalInstructions == 0) {
            out << "No instructions in program, cannot calculate CPI.\n";
            return;
        }   
        out << "Estimated CPI (without hazards): 1.0\n";
        out << "Estimated CPI (with hazards): " << 
               (float)(stats.totalCycles) / stats.totalInstructions << "\n";
        
        if (cacheEnabled) {
            out << "Cache simulation: ENABLED\n";
        }
        
        out << "===========================\n\n";
    }

