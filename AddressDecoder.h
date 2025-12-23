
/**
 * @class AddressDecoder
 * @brief Decodes memory addresses into tag, index, and offset components for cache access.
 *
 * The AddressDecoder class is responsible for extracting the tag, index, and offset
 * fields from a given memory address based on cache configuration parameters.
 * It uses the number of offset and index bits (typically determined by cache line size
 * and number of sets) to compute the bit fields for address decoding.
 *
 * @note This class assumes a 32-bit address space.
 *
 * @see CacheConfig
 */
#ifndef ADDRESS_DECODER_H
#define ADDRESS_DECODER_H

#include "CacheConfig.h"

class AddressDecoder {
private:
    int offsetBits;
    int indexBits;
    int tagBits;

public:
    AddressDecoder():offsetBits(0),indexBits(0),tagBits(0){}

    // Constructor with const reference to avoid unnecessary copy
    AddressDecoder(const CacheConfig& config) {
        offsetBits = config.getOffsetBits();
        indexBits = config.getIndexBits();
        tagBits = 32 - offsetBits - indexBits;
    }

    int getTag(int address) const {
        return address >> (indexBits + offsetBits); 
    }

    int getIndex(int address) const {
        return (address >> offsetBits) & ((1 << indexBits) - 1);
    }

    int getOffset(int address) const {
        return address & ((1 << offsetBits) - 1);
    }
};

#endif // ADDRESS_DECODER_H
