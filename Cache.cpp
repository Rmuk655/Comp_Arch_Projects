    
    /**
     * @file Cache.cpp
     * @brief Implementation of a configurable CPU cache simulator supporting various policies.
     *
     * This file contains the implementation of the Cache class, which simulates a CPU cache with configurable
     * parameters such as block size, cache size, associativity, write policy (Write-Through or Write-Back),
     * and replacement policy (LRU, FIFO, RANDOM). The cache interacts with a simulated main memory and
     * supports both read and write operations, handling cache hits and misses according to the selected policies.
     *
     * Key Features:
     * - Configurable cache parameters via CacheConfig.
     * - Supports direct-mapped, fully associative, and N-way set associative caches.
     * - Implements LRU, FIFO, and RANDOM replacement policies.
     * - Supports Write-Through (WT) and Write-Back (WB) write policies.
     * - Handles cache hits and misses, including block allocation and eviction.
     * - Maintains cache statistics (accesses, hits, misses, hit rate).
     * - Provides logging and cache state dumping for debugging and analysis.
     *
     * Main Classes and Methods:
     * - Cache: Main class representing the cache.
     *   - enable(): Initializes the cache with the given configuration and memory.
     *   - access(): Handles cache accesses (read/write), updating statistics and state.
     *   - handleWriteHit(): Handles write hits according to the write policy.
     *   - handleMiss(): Handles cache misses, including block replacement and allocation.
     *   - replaceBlock(): Selects and replaces a cache block using the configured replacement policy.
     *   - selectBlockToEvict(): Chooses a victim block for eviction.
     *   - findBlockInSet(): Searches for a block with a matching tag in a set.
     *   - writeBlockToMemory(): Writes a cache block back to memory (for WB policy).
     *   - invalidate(): Invalidates all cache blocks, resetting the cache state.
     *   - dump(): Outputs the current cache state for debugging.
     *   - printStats(): Outputs cache access statistics.
     *   - read()/write(): Public API for reading from and writing to the cache.
     *
     * Usage:
     * 1. Create a CacheConfig and Memory instance.
     * 2. Call Cache::enable() to initialize the cache.
     * 3. Use Cache::read() and Cache::write() for memory operations.
     * 4. Use Cache::dump() and Cache::printStats() for diagnostics.
     *
     * Dependencies:
     * - Cache.h: Cache class definition and related types.
     * - BitUtils.h: Utilities for address decoding and bit manipulation.
     * - Memory: Simulated main memory interface.
     *
     * Author: [Your Name]
     * Date: [Date]
     */
    #include <algorithm>
    #include <iostream>
    #include <sstream>
    #include <string>
    #include <cstdlib>   // for std::rand, std::srand
    #include <ctime>     // for time()
    #include <iomanip>

    #include "Cache.h"
    #include "BitUtils.h"


    // READ: Cache Hit	-> Read from cache, update lastUsed for LRU.
    // READ: Cache Miss ->	Fetch from memory, possibly evict block using eviction policy.
    //                      Write old block's data back to memory if it is marked dirty (WB policy)	 
    //                      Allocate new block/ Load from memory, update cache metadata.
    //                      Update Last used, valid, tag etc

    //                              WRITE THROUH + NO ALLOCATE              Write Back + ALLOCATE
    // WRITE: Write Hit	            Write to cache + memory (WT)         	Write to cache, mark dirty (WB)
    // WRITE: Write Miss 	        Write directly to memory                Allocate block, write to cache, mark dirty
    // WRITE:Evict Dirty Block	    N/A (no dirty blocks)	                Write to memory during eviction

    using namespace BitUtils;

    std::string toHexString(uint32_t val) {
        std::ostringstream oss;
        oss << std::hex << val;
        oss << std::dec;
        return oss.str();
    }

    void Cache::logAccess(char op, uint32_t address, uint32_t setIndex,
                        const std::string& hitOrMiss, uint32_t tag, bool dirty) {
        if (logStream && logStream->good()) {
            *logStream << op << ": Address: 0x" << std::hex << address
                    << ", Set: 0x" << setIndex 
                    << ", Tag: 0x" << tag
                    << ", " << (dirty ? "Dirty" : "Clean") 
                    << ", " << hitOrMiss
                    << "\n";
            *logStream << std::dec;
        }
        
       else {
            std::cerr << "[ERROR] logStream badbit: " << logStream->bad()
                      << ", failbit: " << logStream->fail()
                      << ", eofbit: " << logStream->eof() << "\n";
        }         
    }

    // === Constructor ===
    Cache::Cache(): numSets(0),accesses(0), hits(0), misses(0) {
        invalidate();
    }

    // Initializes the cache based on the config.
        
    void Cache::enable(const CacheConfig& config, Memory& mem)   {
        invalidate();
        this->config= config;
        if (config.blockSize == 0 || config.cacheSize == 0) {
            throw std::invalid_argument("Cache and block size must be greater than zero.");
        }

        size_t numBlocks = config.cacheSize / config.blockSize;
        
        int assoc = config.associativity;

        if (config.replacementPolicy == ReplacementPolicy::RANDOM) {
            if (randomSeed == 0) {
                std::srand(static_cast<unsigned int>(time(nullptr)));  // seed rand once
            }
            else {
            // during tests use a fixed seed.
            std::srand(randomSeed);
            }
        }

        if (assoc == 0) {
            // Fully associative: all blocks in one set
            numSets = 1;
            assoc = static_cast<int>(numBlocks);
        } else if (assoc == 1) {
            // Direct mapped: one block per set
            numSets = numBlocks;
            assoc = 1;
        } else {
            // N-way set associative
            numSets = numBlocks / assoc;
        }

        sets.resize(numSets);
        //For every set in the cache (sets), resize it to have assoc blocks (i.e., number of ways in each set).
        // This is where each set is configured to be fully associative (if assoc == numBlocks), 
        // direct-mapped (assoc == 1), or N-way set-associative.
        for (auto& set : sets) {
            set.resize(assoc);
        }

        // Initialize address decoder with updated config
        this->addrDecoder = AddressDecoder(config);
        // Share the same memory location across all of simulation
        this->memory = &mem;

    }


    CacheBlock* Cache::selectBlockToEvict(std::vector<CacheBlock>& set, std::string &policyStr) {
       
        CacheBlock* victim = nullptr;

        if (config.replacementPolicy == ReplacementPolicy::LRU) {
            policyStr = "LRU";
            uint64_t oldestAccess = UINT64_MAX;
            for (auto& block : set) {
                if (block.lastUsed < oldestAccess) {
                    oldestAccess = block.lastUsed;
                    victim = &block;
                }
            }
        } else if (config.replacementPolicy == ReplacementPolicy::FIFO) {
            policyStr = "FIFO";
            uint64_t earliest = UINT64_MAX;
            for (auto& block : set) {
                if (block.insertionTime < earliest) {
                    earliest = block.insertionTime;
                    victim = &block;
                }
            }
        } else if (config.replacementPolicy == ReplacementPolicy::RANDOM) {
            policyStr = "RANDOM";
            int randomIndex = std::rand() % static_cast<int>(set.size());
            victim = &set[randomIndex];
        } else {
            policyStr = "DEFAULT";
            victim = &set[0];  // default fallback; should never be here
        }

        return victim;
    }



    uint32_t Cache::access(uint32_t address, MemSize size, bool isUnsigned, bool isWrite, uint32_t value) {
        uint32_t index = addrDecoder.getIndex(address);
        uint64_t tag  = addrDecoder.getTag(address);
        auto& set = sets[index];
        ++accesses;
        // Search for the block in the set
        CacheBlock* block = findBlockInSet(set, tag);

        if (block) {
            // Cache hit
            hits++;
            // for LRU
            block->lastUsed = accesses;

            // Write Hit: find the cache block corresponding to the address.
            //      Write-Through Policy (WT): Update cache block. Immediately write to main memory too. No dirty bit needed.
            //      Write-Back Policy (WB): Update cache block,  Mark the block as dirty. No memory write now it will happen later, on eviction.
            if (isWrite) {
                // TBD: return address for write?
                return handleWriteHit(*block, index, address, size, value);
            } else {
            // Read Cache Hit -> Read from the cache line.
            // TBD: returning value for read?
                logAccess('R', address, index, "Hit", tag, block->isDirty());  
                return block->readData(address, size, isUnsigned, config.blockSize);
        
            }
        } else {
            // Cache miss - replace a block
            misses++;
            return handleMiss(set, index,address, tag, size, isUnsigned, isWrite, value);
        }
    }


    // Search for the block in the set for a  given tag
    CacheBlock* Cache::findBlockInSet(std::vector<CacheBlock>& set, uint64_t tag) {
        for (auto& block : set) {
            if (block.isValid() && block.tag == tag) {
                return &block;
            }
        }
        return nullptr;
    }

    // Update Cache block for both WT and WB
    // Write-Back Policy (WB): Mark the block as dirty. No memory write now ; it will happen later, on eviction.
    // Write-Through Policy (WT): Immediately write to main memory too. No dirty bit needed.
    
    uint32_t Cache::handleWriteHit(CacheBlock& block, int setIndex, uint32_t address, MemSize size, uint32_t value) {
        
    
        block.writeData(address, size, value, config.blockSize);
        
        if (config.writePolicy == WritePolicy::WB) {
            block.dirty = true;
        } else {
            writeToMemory(address, size, value);
            block.dirty = false;
        }

        std::string policyStr = config.writePolicy == WritePolicy::WB ? "WB" : "WT";
        logAccess('W', address, setIndex, "Hit, " + policyStr, block.tag, block.isDirty());

                        
        return address;
    }

    uint32_t Cache::handleMiss(std::vector<CacheBlock>& set, int setIndex, uint32_t address, uint64_t tag,
                            MemSize size, bool isUnsigned, bool isWrite, uint32_t value) {

        // No-Allocate for WT i.e.  On a write miss, write directly to memory, but do not load the block into the cache.
        if (isWrite && config.writePolicy == WritePolicy::WT) {
            // WT No-Write-Allocate
            writeToMemory(address, size, value);
            logAccess('W', address, setIndex, "Miss, WT Write-through (No Allocation)", tag, false);
            return address;
        }

        CacheBlock* block = replaceBlock(set,setIndex, tag, address,isWrite);
    
        // Write  Allocate for Write Back as per problem statement.
        // means: Write back + allocate on miss -> On a write miss, the block is first loaded from memory to cache, then updated.
        // Note: Memory is updated later when the block is evicted as block is marked as dirty
        //if(config.writePolicy == WritePolicy::WB)
        if (isWrite) {
            block->writeData(address, size, value, config.blockSize);
            block->markDirty();
            logAccess('W', address, setIndex, "Miss, WB Write-back with Allocation", tag, block->isDirty());
            return address;
        } else {
        // Read the requested data from the new block in cache. 
            logAccess('R', address, setIndex, "Miss, Read Allocated Block (WB or WT)", tag, block->isDirty());               
            return block->readData(address, size, isUnsigned, config.blockSize);
        }

    }

    //Select a block to evict using replacement policy (LRU, FIFO, etc.).
    // If the victim block is dirty, and your policy is Write-Back, you must:
    // Write the evicted block’s data back to memory before replacing.
    // Load the new block from memory into cache.
    // Update lastUsed (LRU),insertionTime(FIGF) valid, tag, etc.
    // Return the new block with correct data

    CacheBlock* Cache::replaceBlock(std::vector<CacheBlock>& set, int setIndex, uint64_t tag, uint32_t address, bool isWrite) {
        // Step 1: Try to find an invalid block first
        CacheBlock* victim = nullptr;
        std::string policyStr = "";
         
        for (auto& block : set) {
            if (!block.isValid()) {
                victim = &block;
                policyStr = "INVALID_BLOCK";
                break;
            }
        }

        // Step 2: If all blocks are valid, select one to evict
        if (!victim) {
            victim = selectBlockToEvict(set,policyStr);

            // Step 3: Write back if needed (Write-Back + Dirty)
            if (victim->dirty && config.writePolicy == WritePolicy::WB) {
                //uint32_t victimAddr = ((victim->tag << config.getIndexBits()) | setIndex) << config.getOffsetBits();
                writeBlockToMemory(*victim);

                
            }
            if (victim->dirty && config.writePolicy == WritePolicy::WB) {
            logAccess('W', address, setIndex, "Evicting dirty block (WB)", victim->tag, true);
            } else if (victim->isValid()) {
                logAccess('W', address, setIndex, "Evicting clean block", victim->tag, false);
            }
        }

        

        // Step 4: Reset and load new block
        // Clear previous data
        victim->data.clear();
        victim->markValid();
        victim->tag = tag;
        victim->index = setIndex;
        victim->dirty = (isWrite && config.writePolicy == WritePolicy::WB);
        victim->lastUsed = accesses;
        victim->insertionTime = accesses;
        victim->data.resize(config.blockSize);

        // Step 5: Load block data from memory
        uint32_t blockAddr = address & ~(config.blockSize - 1);
        for (int i = 0; i < config.blockSize; ++i) {
            victim->data[i] = memory->read(blockAddr + i, MemSize::Byte);
        }
            
        logAccess(isWrite?'W':'R', address, setIndex, "Miss, Replacing block in set using " + policyStr, tag, victim->isDirty());              
        return victim;
    }

    void Cache::writeBlockToMemory(const CacheBlock& block) {
        uint32_t blockAddr = block.getBlockAddress(config);
        block.writeBackToMemory(*memory, blockAddr);
    }

    void Cache::invalidate() {
        for (auto& set : sets) {
            for (auto& block : set) {
                block.markInvalid() ;    // Without this, the cache might falsely treat garbage data as valid.
                block.markClean();   // throwing away all data, so it makes no sense to consider any block "dirty".
                block.tag = 0;          //since valid = false, the tag is irrelevant, but zeroing it is good hygiene
                block.lastUsed = 0;     // resetting to say this block has never been used yet
            }
        }
        accesses = 0;
        hits = 0;
        misses = 0;
    }

    void Cache::dump(std::ostream& out) const {

    for (size_t i = 0; i < sets.size(); ++i) {
        bool hasValidBlock = false;
        for (const auto& block : sets[i]) {
            if (block.isValid()) {
                hasValidBlock = true;
                break;
            }
        }

        if (hasValidBlock) {
            out << "Set[" << std::dec << i << "]:\n";
            for (const auto& block : sets[i]) {
                if (block.isValid()) {
                    out << "  " << block << "\n";
                }
            }
        }
    }
        out << std::dec;
    }

    void Cache::printStats(std::ostream& out) const {
        out << std::dec;
        out << "D-cache statistics: ";
        out << "  Accesses: " << accesses << " ";
        out << "  Hits: " << hits << " ";
        out << "  Misses: " << misses << " ";
        if (accesses > 0) {
            double hitRate = static_cast<double>(hits) / accesses * 100.0;
            out << "  Hit Rate: " << hitRate ;
        }
        out << "%\n";
    }


    uint32_t Cache::read(uint32_t address, MemSize size, bool isUnsigned) {
        return access(address, size, isUnsigned, false );
    }

    void Cache::write(uint32_t address, MemSize size, uint32_t value) {
        
        access(address, size, false, true, value );
    }

    void Cache::writeToMemory(uint32_t address, MemSize size, uint32_t value) {
        if(memory) memory->write(address,size,value);    
    }


    int Cache::readFromMemory(uint32_t address, MemSize size, bool isUnsigned ) const {
        if(memory) return memory->read(address,size,isUnsigned);    
        return -1;
    }
