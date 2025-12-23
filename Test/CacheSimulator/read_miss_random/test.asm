# Scenario: READ - Cache Miss (Random)
# Expected: Block loaded, random eviction if needed.
    # Initialize base address to 0x1000
    lui   t0, 0x1          # t0 = 0x1000

    # Store 123 at 0x1000 (memory write)
    addi  t1, zero, 123
    sw    t1, 0(t0)

    # Access blocks to fill cache and cause eviction
    lw    t1, 0(t0)        # Access block 1 - MISS → insert block 1 into set
    lw    t2, 512(t0)      # Access block 2 - MISS → insert block 2 into set

    # Now cache set is full (2 blocks), next access causes eviction

    lw    t3, 1024(t0)     # Access block 3 - MISS → triggers RANDOM eviction of one block in set

    # Access some blocks again to test eviction effects

    lw    t4, 512(t0)      # Access block 2 - HIT if not evicted, MISS otherwise
    lw    t5, 0(t0)        # Access block 1 - HIT if not evicted, MISS otherwise

    # At this point, cache contains 2 blocks chosen randomly by eviction policy
