
/**
 * @file CacheBlock.h
 * @brief Defines the CacheBlock class representing a single block in a CPU cache.
 *
 * A CacheBlock models the behavior and state of a cache line in a CPU cache system.
 * It stores data, tag, validity, dirtiness, and metadata for replacement policies.
 * 
 * Usage:
 * - When a CPU requests data, the cache uses the address to extract the index and tag.
 * - The cache checks the valid bit and compares the tag to determine a hit or miss.
 * - On a miss, if the block is dirty, it must be written back to memory before replacement.
 * 
 * Main Features:
 * - Read and write data at specific offsets within the block.
 * - Track validity and dirtiness of the block.
 * - Support for LRU and insertion time metadata.
 * - Methods for writing back dirty blocks to main memory.
 * - Overloaded stream operator for easy block state visualization.
 *
 * Dependencies:
 * - Memory.h: Interface for main memory operations.
 * - CacheConfig.h: Provides cache configuration parameters (index bits, offset bits, etc.).
 */

 /**
    * @class CacheBlock
    * @brief Represents a single cache block (cache line) in a CPU cache.
    *
    * Members:
    * - valid: Indicates if the block contains valid data.
    * - dirty: Indicates if the block has been modified (write-back required).
    * - tag: Tag bits for address comparison.
    * - lastUsed: Timestamp or counter for LRU replacement policy.
    * - insertionTime: Timestamp for FIFO or other policies.
    * - data: Byte vector storing the actual block data.
    * - index: Set index for reconstructing the memory address.
    *
    * Public Methods:
    * - writeData: Write a value to the block at a given address and size.
    * - readData: Read a value from the block at a given address and size.
    * - getBlockAddress: Reconstruct the memory address this block maps to.
    * - writeBackToMemory: Write the block's data back to main memory if dirty.
    * - isValid, isDirty, markValid, markInvalid, markDirty, markClean: State management.
    *
    * Operators:
    * - operator<<: Stream output for debugging and visualization.
    */
#ifndef CACHE_BLOCK_H
#define CACHE_BLOCK_H

/* A CPU requests data from memory address 0xABCDEF1234:
The cache extracts the index to find the right set or block.
It checks the valid bit of the block at that index.
If valid == true, it compares the extracted tag from the address with the block's stored tag.
If they match, it is a cache hit.
If they do not match, it is a miss, and the cache block might be updated.
If the block being replaced is dirty, it must be written back to memory before replacement.
*/

#include <cstdint>
#include <vector>
#include <iomanip>
#include "Memory.h"
#include "CacheConfig.h"

class CacheBlock {
public:
 
    CacheBlock() : valid(false), dirty(false), tag(0), lastUsed(0), insertionTime(0),index(0) {}
    // Writes value into this block at the correct offset based on the memory address.
    // Assumes this block starts at a given base address (block-aligned).
    void writeData(uint32_t address, MemSize size, uint32_t value, uint32_t blockSize);
    // Reads value from this block at the given address offset.
    uint32_t readData(uint32_t address, MemSize size,   bool isUnsigned, uint32_t blockSize) const;
    // method to get the memory address this block maps to
    uint32_t getBlockAddress(const CacheConfig& config) const {
        uint32_t blockAddr = ((tag << config.getIndexBits()) | index) << config.getOffsetBits();
        return blockAddr;
    }

    void writeBackToMemory(Memory& memory, uint32_t blockStartAddress) const;

    inline bool isValid() const { return valid;}

    inline bool isDirty() const { return dirty;}

    inline void markValid()  { valid = true;}

    inline void markInvalid()  { valid = false;}

    inline void markDirty() { dirty = true; }

    inline void markClean() { dirty = false; }

    CacheBlock(const CacheBlock&) = default;
    CacheBlock& operator=(const CacheBlock&) = default;
    CacheBlock(CacheBlock&&) noexcept = default;
    CacheBlock& operator=(CacheBlock&&) noexcept = default;


    bool valid; //Indicates whether the contents of the cache block are valid
    bool dirty; //Indicates whether the block has been written to but not yet written back to main memory.
    uint64_t tag;
    uint64_t lastUsed; // For LRU policy
    uint64_t insertionTime;
    std::vector<uint8_t> data;  // blockSize bytes
    uint32_t index; // Set index to help reconstruct address

};

inline std::ostream& operator<<(std::ostream& os, const CacheBlock& block) {
    if (!block.valid) {
        os << "[Invalid Block]";
        return os;
    }

    os << "Index: 0x" << std::hex << std::setw(2) << std::setfill('0') << block.index
       << " | Tag: 0x" << std::setw(8) << block.tag
       << " | " << (block.dirty ? "Dirty" : "Clean")
       << " | Data: [";

    for (size_t i = 0; i < block.data.size(); ++i) {
        if (i > 0) os << " ";
        os << std::hex << std::setw(2) << std::setfill('0') << static_cast<int>(block.data[i]);
    }

    os << "]";
    os << std::dec;
    return os;
}
#endif // CACHE_BLOCK_H
