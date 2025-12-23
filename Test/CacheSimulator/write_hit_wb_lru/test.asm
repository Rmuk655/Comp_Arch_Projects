# Scenario: WRITE HIT - WB + Allocate (LRU)
# Expected: Write to cache, mark dirty.
     # Initialize base addresses
    lui t0, 0x1000       # t0 = 0x1000_0000  (some aligned block)
    lw  t1, 0(t0)        # Load word from 0(t0), causes a cache miss and block fill
    addi t2, zero, 42    # t2 = 42
    sw  t2, 0(t0)        # Write to same block - should be a write hit, cache block becomes dirty

    # Access another block in same set to test LRU
    lui t3, 0x1000       # Same high bits to ensure same set (associativity 2)
    addi t3, t3, 16      # Next block (block offset + 16 bytes)
    lw  t4, 0(t3)        # Miss, block loaded in cache

    # Access first block again to make sure it's LRU updated
    lw  t5, 0(t0)        # Hit

    # Write to yet another block in same set to cause eviction based on LRU
    lui t6, 0x1000
    addi t6, t6, 32      # Third block in same set (eviction will happen)
    sw  t2, 0(t6)        # <-- Use t2 or another valid register here

    # Program end
