
/**
 * @file BitUtils.h
 * @brief Utility functions for bit manipulation operations.
 *
 * This header provides a set of inline functions for common bitwise operations,
 * such as counting trailing zeros, extracting and packing bitfields, computing
 * log2 for powers of two, and finding the highest power of two less than or equal to a given value.
 *
 * Namespace: BitUtils
 *
 * Functions:
 * - int countTrailingZeros(uint32_t x):
 *     Counts the number of trailing zero bits in the given unsigned 32-bit integer.
 *     Returns 32 if x is 0.
 *
 * - uint32_t extract(uint32_t value, int start, int length):
 *     Extracts a bitfield of 'length' bits starting from bit position 'start' from 'value'.
 *     Example: extract(0b101110, 1, 3) → 0b111
 *
 * - uint32_t pack(uint32_t value, int start, int length):
 *     Packs the lower 'length' bits of 'value' into a field starting at bit position 'start'.
 *     Example: pack(0b101, 4, 3) → 0b1010000
 *
 * - int log2(uint32_t x):
 *     Returns the number of bits needed to represent a number, assuming x is a power of 2.
 *     Effectively computes the base-2 logarithm of x.
 *
 * - uint32_t floorPowerOfTwo(uint32_t x):
 *     Returns the highest power of two less than or equal to x.
 *     Returns 0 if x is 0.
 */
#ifndef BIT_UTILS_H
#define BIT_UTILS_H

#include <cstdint>

namespace BitUtils {

    // Count trailing zeros in an unsigned integer (basic version of __builtin_ctz)
    inline int countTrailingZeros(uint32_t x) {
        if (x == 0) return 32; // You can handle this case differently if needed
        int count = 0;
        while ((x & 1) == 0) {
            x >>= 1;
            count++;
        }
        return count;
    }

    // Extract 'length' bits starting from bit position 'start'
    // Example: extract(0b101110, 1, 3) → 0b111
    inline uint32_t extract(uint32_t value, int start, int length) {
        return (value >> start) & ((1u << length) - 1);
    }

    // Pack 'value' into 'length' bits starting at position 'start'
    // Example: pack(0b101, 4, 3) → 0b1010000
    inline uint32_t pack(uint32_t value, int start, int length) {
        return (value & ((1u << length) - 1)) << start;
    }

    // Get the number of bits needed to represent a number (assumes power of 2)
    inline int log2(uint32_t x) {
        int count = 0;
        while (x > 1) {
            x >>= 1;
            count++;
        }
        return count;
    }

    // Returns the highest power of two <= x
    inline uint32_t floorPowerOfTwo(uint32_t x) {
        if (x == 0) return 0;
        uint32_t result = 1;
        while (result << 1 <= x) {
            result <<= 1;
        }
        return result;
    }

}



#endif // BIT_UTILS_H
