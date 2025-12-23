    # Scenario: READ - Cache Miss (LRU)
    # Expected: Block loaded, possible eviction based on LRU.
    # Scenario: READ - Cache Miss (LRU)
    # Expected: Block loaded, possible eviction based on LRU.
    # Initialize base address
    lui   t0, 0x1          # t0 = 0x1000

    addi  t1, zero, 123
    sw    t1, 0(t0)        # store 123 at 0x1000 (memory write)

    # Accesses simulating LRU cache behavior:

    lw    t1, 0(t0)        # Access block 1 - MISS → insert block 1 into set
    lw    t2, 512(t0)      # Access block 2 - MISS → insert block 2 into set
    lw    t3, 1024(t0)     # Access block 3 - MISS → evict least recently used block (block 1)
    
    # Now block 2 and block 3 are in cache; block 1 was evicted

    lw    t4, 512(t0)      # Access block 2 again - HIT → updates LRU order (block 3 is now LRU)
    lw    t5, 0(t0)        # Access block 1 - MISS → block 3 (LRU) evicted, reload block 1

    # Final cache state: block 1 and block 2 present, block 3 evicted
