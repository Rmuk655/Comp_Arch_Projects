
#ifndef CACHE_CONFIG_H
#define CACHE_CONFIG_H
/* 
cacheSize =Total size of the cache.
blockSize = Size of each block (also called a cache line) that the cache stores.
associativity = Number of blocks in each set of the cache.
ReplacementPolicy -> decides which block to evict when a set is full and new data needs to be loaded.
    FIFO: Replace the block that entered the set earliest.
    LRU: Replace the block that was used least recently.
    RANDOM: Replace a random block in the set.
writePolicy ->Controls how writes to memory are handled when a cache block is modified.
    WB (Write-Back): Data is written to memory only when block is evicted. Modified blocks are marked dirty.
    WT (Write-Through): Data is written immediately to main memory every time it is updated in the cache.
loadFromFile(filename):Reads cache parameters from a config file
e.g: cacheSize=8192
blockSize=64
associativity=4
replacementPolicy=LRU
writePolicy=WB

*/


#include <string>
#include <iostream>

enum class ReplacementPolicy { FIFO, LRU, RANDOM };
enum class WritePolicy { WB, WT };

class CacheConfig {
public:
    int cacheSize;            // Total cache size in bytes (max 1 MB, power of 2)
    int blockSize;            // Block size in bytes (max 64, power of 2)
    int associativity;        // Associativity (1=direct mapped, 0=fully associative, else set assoc up to 16)
    ReplacementPolicy replacementPolicy;
    WritePolicy writePolicy;

    CacheConfig()=default;

    // Loads config parameters from the given file
    // Format (one per line):
    // SIZE_OF_CACHE
    // BLOCK_SIZE
    // ASSOCIATIVITY
    // REPLACEMENT_POLICY (FIFO, LRU, RANDOM)
    // WRITEBACK_POLICY (WB, WT)
    //
    // Returns true if successful, false otherwise
    bool loadFromFile(const std::string& filename);
    int getOffsetBits() const ;

    int getIndexBits() const ;
private:
    bool isPowerOfTwo(int x) const;
};


// Declarations only
std::string replacementPolicyToString(ReplacementPolicy policy);
std::string writePolicyToString(WritePolicy policy);
std::ostream& operator<<(std::ostream& os, const CacheConfig& config);




#endif // CACHE_CONFIG_H
