

/**
 * @class Cache
 * @brief Simulates a configurable cache memory system.
 *
 * The Cache class models the behavior of a CPU cache, supporting configurable parameters
 * such as associativity, block size, and number of sets via the CacheConfig object.
 * It manages cache blocks, tracks access statistics (hits, misses, total accesses),
 * and provides methods for cache access, invalidation, dumping state, and statistics reporting.
 *
 * @param config      The cache configuration, specifying size, associativity, and block size.
 * @param numSets     The number of sets in the cache, derived from the configuration.
 * @param sets        The cache storage, organized as a vector of sets, each containing cache blocks.
 * @param accessCounter  A counter to track the number of accesses, useful for replacement policies.
 * @param accesses    The total number of cache accesses.
 * @param hits        The number of cache hits.
 * @param misses      The number of cache misses.
 */

#ifndef CACHE_H
#define CACHE_H


#include <vector>
#include <cstdint>
#include <fstream>
#include <string>
#include "CacheConfig.h"
#include "CacheBlock.h"
#include "Memory.h"
#include "AddressDecoder.h"


class Cache {
public:
    Cache();
    // Initializes the cache based on the config.
    void enable(const CacheConfig& cfg, Memory& mem);
    // Resets statistics and access counter.
    void reset();

    uint32_t read(uint32_t address, MemSize size, bool isUnsigned = false);
    void write(uint32_t address, MemSize size, uint32_t value);

    // Simulates a read or write operation:
    //      Increments accessCounter and accesses. Looks up the correct set and tag.
    //      If found → Hit. If not → Miss, replace block using policy.
    //      If isWrite and policy is Write-Back, mark block dirty.
    uint32_t access(uint32_t address, MemSize size, bool isUnsigned, bool isWrite, uint32_t writeValue=0);
    // Clears the cache (marks all blocks invalid).
    void invalidate();
    //Dump the current cache state to a outputstream (like a memory dump).
    void dump(std::ostream& out) const;
    //Displays performance
    void printStats(std::ostream& out) const;

    void writeToMemory(uint32_t address, MemSize size, uint32_t value) ;
    int readFromMemory(uint32_t address, MemSize size, bool isUnsigned = false) const;
    void setLogStream(std::ostream& log) { logStream = &log;}

    
    // Set the seed for the random number generator (for testing)
    void setRandomSeed(uint32_t seed) {
        randomSeed = seed;
        std::srand(randomSeed);
    }

    private:
        // CacheSimulator should initialize these
        std::ostream* logStream = nullptr;
        Memory *memory = nullptr;
        CacheConfig config;
        uint32_t randomSeed = 0; // 0 means "not set", use time-based default which will be real random

        AddressDecoder addrDecoder;
        //the number of rows: numSets = (cacheSize / blockSize) / associativity;
        int numSets;
        // Each sets[i] is a row of lockers (a set), containing associativity number of CacheBlocks.
        //sets[0] = {block1, block2, block3, block4};  -> 4 blocks in set 0
        std::vector<std::vector<CacheBlock>> sets;
        //A global counter that increases every time the cache is accessed (read or write).
        //Used in LRU replacement: Helps determine which block was least recently used by comparing their lastUsed timestamps.
        int accesses;

        // Performance Statistics
        //    accesses: Total number of times the cache was accessed (read/write).
        //    hits: Number of times the data was found in the cache.
        //    misses: Number of times the data was not found, and had to be fetched from memory.
        // try to open cache block with tag X in set Y.
        //          If the cache is valid and contains the tag: Hit.
        //          Else: Miss, and the cache has to bring data from memory and possibly evict an existing block.

        int hits;
        int misses;

        // Helper functions to extract index, Tag etc
        int getSetIndex(int address) const;
        int getTag(int address) const;
        //Checks if a block with a specific tag exists in the given set. Returns its index if found (i.e., hit), or -1 (i.e., miss).
        int findBlockIndex(int setIndex, int tag) const;
        // Chooses which block in a full set to evict. Based on the ReplacementPolicy:
        //        FIFO: First one inserted.
        //        LRU: Least recently used (lastUsed is smallest).
        //        RANDOM: Any block.
        
        CacheBlock* selectBlockToEvict(std::vector<CacheBlock>& set,std::string &policyStr) ;

        std::pair<uint32_t, uint64_t> decodeAddress(uint32_t address) const;
        CacheBlock* findBlockInSet(std::vector<CacheBlock>& set, uint64_t tag);
        uint32_t handleWriteHit(CacheBlock& block, int setIndex, uint32_t address, MemSize size, uint32_t value) ;
        uint32_t handleMiss(std::vector<CacheBlock>& set, int setIndex, uint32_t address, uint64_t tag,
                            MemSize size, bool isUnsigned, bool isWrite, uint32_t value);
        CacheBlock* replaceBlock(std::vector<CacheBlock>& set, int setIndex, uint64_t tag, uint32_t address,bool isWrite);
        void writeBlockToMemory(const CacheBlock& block) ;

        void logAccess(char op, uint32_t address, uint32_t setIndex, const std::string& hitOrMiss, uint32_t tag, bool dirty);
};

#endif // CACHE_H
