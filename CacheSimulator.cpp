#include "CacheSimulator.h"
#include <iostream>
#include <fstream>
#include <sstream>
#include <filesystem>


// Check if cache simulation is enabled.
// Look up the cache to determine a hit or miss.
// On a miss, decide whether to load the block from memory into the cache (according to policy).
// For writes, depending on the write policy (WB or WT), update the cache and/or memory.
// Log access to the filename.output trace file. Update statistics.
// Mainly orchestration + logging + lifecycle control only.

// Memory has to be set by Simulator; common memory fragment for all classes
bool CacheSimulator::enable(const std::string& configFile, Memory& mem) {
    CacheConfig cfg;
    if (!cfg.loadFromFile(configFile)) {
        std::cerr << "Failed to load cache configuration.\n";
        return false;
    }
    
    config = cfg;
    cache.enable(config, mem);
    enabled = true;

    return true;
}


void CacheSimulator::disable() {
    enabled = false;
    if (logFile.is_open()) {
        logFile.close();
    }
}

void CacheSimulator::setProgramName(const std::string& programName) {
    currentProgramName = programName;
    if (!enabled) return; // dont create log files if simulator is not enabled.
    // If logFile was already open, close it before reopening
    if (logFile.is_open()) {
        logFile.close();
    }

    logFile.open(currentProgramName + ".log",std::ios::out | std::ios::trunc);

    
    if (!logFile) {
        std::cerr << "Failed to open log file.\n";
        
    }

    
    if (logFile.is_open()) {
        cache.setLogStream(logFile);
    } else {
        std::cerr << "[CacheSimulator] Log file open failed. Not setting log stream.\n";
    }
  

}


uint32_t CacheSimulator::read(uint32_t address, MemSize size, bool isUnsigned) {
    if (!enabled ) return address;
    return cache.read(static_cast<uint64_t>(address), size, isUnsigned);
    
}

void CacheSimulator::write(uint32_t address, MemSize size, uint32_t value) {
      if (!enabled ) return;
    cache.write(static_cast<uint64_t>(address), size, value);
    
    
}

void CacheSimulator::invalidate() { 
    
    if (logFile.is_open()) logFile.flush();
    cache.invalidate();
    // dont close log fie; it is causing corruption
    //if (logFile.is_open()) logFile.close();
     
}

void CacheSimulator::dump(std::ostream& out) const {
        cache.dump(out);
}


void CacheSimulator::dump(const std::string& filename) const {
    if (!enabled) return;
    std::ofstream out(filename);
    if (out.is_open()) {
        dump(out);
    }
    else {
        std::cerr << "Failed to open file: " << filename << std::endl;
    }
}
    


void CacheSimulator::printStatus(std::ostream& out) const {
    if (enabled) {
        out << "Cache Simulation Status: Enabled\n";
        out << config;  
    } else {
        out << "Cache Simulation Status: Disabled\n";
    }
}

void CacheSimulator::printStats(std::ostream& out) const {
    if (!enabled) return;
    out << "Program: " << currentProgramName << "\n";
    cache.printStats(out);
}
