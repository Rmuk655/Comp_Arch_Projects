#ifndef CACHE_SIMULATOR_H
#define CACHE_SIMULATOR_H
//CacheSimulator  runs the simulation of a cache. 
//      Turns the cache on/off (via config file)
//      Tells the cache when a read/write happens
//      Collects stats
//      Allows invalidating or dumping the cache


#include "Cache.h"
#include "CacheConfig.h"
#include <string>
#include <memory>
#include <fstream>
#include "Memory.h"

class CacheSimulator {
private:
    //The actual cache being simulated
    Cache cache;
    // Stores the settings like size, block, policy, etc.
    CacheConfig config;
    //Indicates if the cache simulation is currently active
    bool enabled;
    //File to write logs about each memory access
    std::ofstream logFile;
    //which "program" is running in simulation
    std::string currentProgramName;

    
public:
    CacheSimulator():enabled(false){};
    //Loads config, creates cache instance
    bool enable(const std::string& configFile, Memory& mem);
    // Disables simulation, cleans up
    void disable();
    //set current app name for logs/stats
    void setProgramName(const std::string& programName);

    // Read returns data
    uint32_t read(uint32_t addr, MemSize size, bool isUnsigned);


    // Write passes data
    void write(uint32_t address, MemSize size, uint32_t value);
    // Clears the entire cache
    void invalidate();
    //Dumps current cache content to a file
    void dump(const std::string& filename) const;

    //Dumps current cache content to output stream
    void dump(std::ostream& out) const;


    // Prints hit/miss statistics to the provided output stream
    void printStats(std::ostream& out) const;

    // Prints current status to the provided output stream
    void printStatus(std::ostream& out) const;

    
    // is simulation active?
    bool isEnabled() const { return enabled; }

    // Set the seed for the random number generator (for testing)
    void setRandomSeed(uint32_t seed) {
            cache.setRandomSeed(seed);
    }

       
};

#endif // CACHE_SIMULATOR_H
