
/**
 * @file Memory.cpp
 * @brief Implementation of the Memory class for simulating memory operations.
 *
 * This file provides the implementation for the Memory class, which allows for
 * reading, writing, clearing, and printing memory contents. The memory is modeled
 * as a byte-addressable storage, supporting different data sizes (Byte, HalfWord, Word)
 * and both signed and unsigned accesses.
 *
 * Functions:
 * - void Memory::clear()
 *      Clears all memory contents.
 *
 * - int32_t Memory::read(uint32_t address, MemSize size, bool isUnsigned) const
 *      Reads a value from memory at the specified address and size.
 *      Supports sign-extension for signed accesses.
 *
 * - void Memory::write(uint32_t address, MemSize size, uint32_t value)
 *      Writes a value to memory at the specified address and size.
 *
 * - void Memory::print(uint32_t startAddress, uint32_t count) const
 *      Prints the contents of memory starting from a given address for a specified count.
 */
#include "Memory.h"


void Memory::clear()
{
    memory.clear();
}

int32_t Memory::read(uint32_t address, MemSize size, bool isUnsigned) const {

    int sz = static_cast<int>(size);
    uint32_t val = 0;
    for (int i = 0; i < sz; ++i) {
        val |= (static_cast<uint32_t>(static_cast<uint8_t>(getByte(address + i))) << (8 * i));
    }

    // Sign-extend for smaller sizes if needed
    if (!isUnsigned) {
        if (size == MemSize::HalfWord) {
            return static_cast<int16_t>(val);
        }
        if (size == MemSize::Byte) {
            return static_cast<int8_t>(val);
        }
    }

    return val;
}

void Memory::write(uint32_t address,  MemSize size, uint32_t value) {
    

    int sz = static_cast<int>(size);
    for (int i = 0; i < sz; ++i) {
        memory[address + i] = static_cast<int8_t>((value >> (8 * i)) & 0xFF);
    }
}

void Memory::print(uint32_t startAddress, uint32_t count) const {
    for (uint32_t i = 0; i < count; ++i) {
        uint32_t addr = startAddress + i * static_cast<int>(MemSize::Word);
        uint32_t val = 0;
        for (int j = 0; j < static_cast<int>(MemSize::Word); ++j) {
            val |= (static_cast<uint32_t>(static_cast<uint8_t>(getByte(addr + j))) << (8 * j));
        }
        out << "Memory[0x" << std::hex << addr << "] = 0x" << val << std::dec << "\n";
    }
}
