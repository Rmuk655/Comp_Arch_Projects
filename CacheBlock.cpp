
/**
 * @file CacheBlock.cpp
 * @brief Implementation of the CacheBlock class for simulating cache memory blocks.
 *
 * This file contains the implementation of methods for reading, writing, and writing back data
 * in a cache block, supporting different memory sizes and sign extension.
 */

#include "CacheBlock.h"

// Writes value into this block at the correct offset based on the memory address.
// Assumes this block starts at a given base address (block-aligned).

// Reads value from this block at the given address offset.
    

void CacheBlock::writeData(uint32_t address, MemSize Msize, uint32_t value, uint32_t blockSize) {
    uint32_t offset = address % blockSize;
    int size = static_cast<int>(Msize);
    if (offset + size > data.size()) return; // Prevent overflow

    for (int i = 0; i < size; ++i) {
        data[offset + i] = static_cast<uint8_t>((value >> (8 * i)) & 0xFF);
    }
    valid = true;
}

// Reads value from this block at the given address offset.
uint32_t CacheBlock::readData(uint32_t address, MemSize Msize,   bool isUnsigned, uint32_t blockSize) const {
    uint32_t offset = address % blockSize;
    int size = static_cast<int>(Msize);
    uint32_t result = 0;
    for (int i = 0; i < size; ++i) {
        if (offset + i < data.size()) {
            result |= static_cast<uint32_t>(data[offset + i]) << (8 * i);
        }
    }

    if (!isUnsigned && size < 4) {
        // Sign-extend if needed
        int32_t signedVal = static_cast<int32_t>(result << ((4 - size) * 8));
        return static_cast<uint32_t>(signedVal >> ((4 - size) * 8));
    }

    return result;
}

//Writes each byte of the block to memory at the correct offset with MemSize::Byte.
void CacheBlock::writeBackToMemory(Memory& memory, uint32_t baseAddress) const {
    if (!valid) return;

    for (int i = 0; i < (int) data.size(); ++i) {
        memory.write(baseAddress + i, MemSize::Byte, data[i]);
    }
}
