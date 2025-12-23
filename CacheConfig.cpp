// Reads exactly 5 lines in order as per your example config.
//Validates power-of-two constraints and value ranges.
// For associativity: 
//          0 means fully associative → sets = 1, blocks per set = total blocks. 1 means direct mapped.
//          Others must be powers of two and divide total blocks.
//Replacement and write policies are case-insensitive.
//Provides error messages for invalid config.
//Sets default values in constructor for safety.

/*
The cache configuration is provided as an input file (e.g., cache.config) in the following format:
SIZE_OF_CACHE (number)
BLOCK_SIZE (number)
ASSOCIATIVITY (number)
REPLACEMENT_POLICY (FIFO or LRU or RANDOM)
WRITEBACK_POLICY (WB or WT)
Example config file:
32168
16
8
LRU
WT
*/

#include "CacheConfig.h"
#include "BitUtils.h"
#include <fstream>
#include <iostream>
#include <algorithm>

#include <iostream>
using namespace BitUtils;


// Helper to print ReplacementPolicy as string
std::string replacementPolicyToString(ReplacementPolicy p) {
    switch (p) {
        case ReplacementPolicy::FIFO: return "FIFO";
        case ReplacementPolicy::LRU: return "LRU";
        case ReplacementPolicy::RANDOM: return "RANDOM";
        default: return "UNKNOWN";
    }
}

// Helper to print WritePolicy as string
std::string writePolicyToString(WritePolicy w) {
    switch (w) {
        case WritePolicy::WB: return "WB";
        case WritePolicy::WT: return "WT";
        default: return "UNKNOWN";
    }
}

// Overload operator<< to print CacheConfig
std::ostream& operator<<(std::ostream& os, const CacheConfig& config) {
    os << std::dec;
    os << "Cache Size: " << config.cacheSize << "\n";
    os << "Block Size: " << config.blockSize << "\n";
    os << "Associativity: " << config.associativity << "\n";
    os << "Replacement Policy: " << replacementPolicyToString(config.replacementPolicy) << "\n";
    os << "Write Back Policy: " << writePolicyToString(config.writePolicy) << "\n";
    return os;
}

bool CacheConfig::isPowerOfTwo(int x) const {
    return x > 0 && (x & (x - 1)) == 0;
}

bool CacheConfig::loadFromFile(const std::string& filename) {
    std::ifstream file(filename);
    if (!file) {
        std::cerr << "Error opening config file: " << filename << "\n";
        return false;
    }

    int sizeOfCache, blkSz, assoc;
    std::string replPolicyStr, writePolicyStr;

    if (!(file >> sizeOfCache)) return false;
    if (!(file >> blkSz)) return false;
    if (!(file >> assoc)) return false;
    if (!(file >> replPolicyStr)) return false;
    if (!(file >> writePolicyStr)) return false;

    // Validate and assign
    if (sizeOfCache <= 0 || sizeOfCache > 1024 * 1024) {
        std::cerr << "Cache size must be > 0 and <= 1MB\n";
        return false;
    }

    // if Cache Size is not a power of 2, use the highest power lower than cache size
    if (!isPowerOfTwo(sizeOfCache)) {
        int original = sizeOfCache;
        sizeOfCache = BitUtils::floorPowerOfTwo(sizeOfCache);
        std::cerr << "Warning: Cache size " << original
              << " is not a power of two. Using "
              << sizeOfCache << " instead.\n";
}
    if (assoc < 0 || assoc > 16 || (assoc != 0 && !isPowerOfTwo(assoc))) {
        std::cerr << "Associativity must be 0 (fully associative) or power of two <= 16\n";
        return false;
    }

    // Map replacement policy string
    if (replPolicyStr == "FIFO") replacementPolicy = ReplacementPolicy::FIFO;
    else if (replPolicyStr == "LRU") replacementPolicy = ReplacementPolicy::LRU;
    else if (replPolicyStr == "RANDOM") replacementPolicy = ReplacementPolicy::RANDOM;
    else {
        std::cerr << "Unknown replacement policy: " << replPolicyStr << "\n";
        return false;
    }

    // Map write policy string
    if (writePolicyStr == "WB") writePolicy = WritePolicy::WB;
    else if (writePolicyStr == "WT") writePolicy = WritePolicy::WT;
    else {
        std::cerr << "Unknown write policy: " << writePolicyStr << "\n";
        return false;
    }

    cacheSize = sizeOfCache;
    blockSize = blkSz;
    associativity = assoc;

    return true;
}



int CacheConfig::getOffsetBits() const {
    return BitUtils::log2(blockSize);
}

int CacheConfig::getIndexBits() const {
    if (associativity == 0) return 0; // Fully associative
    int numSets = (cacheSize / blockSize) / associativity;
    return BitUtils::log2(numSets);
}