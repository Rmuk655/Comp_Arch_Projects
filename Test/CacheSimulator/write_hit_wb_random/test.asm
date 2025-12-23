# Scenario: WRITE HIT - WB + Allocate (Random)
# Expected: Write to cache, mark dirty.
#  .text
    # Load block with Tag A (Set 0)
    lui t0, 0x00000       # t0 = 0x00000000
    lw  t1, 0(t0)         # Miss, block with Tag A loaded into cache set 0

    # Load block with Tag B (Set 0)
    lui t0, 0x00200       # t0 = 0x00200000
    lw  t2, 0(t0)         # Miss, block with Tag B loaded into cache set 0

    # Load block with Tag C (Set 0)
    lui t0, 0x00400       # t0 = 0x00400000
    lw  t3, 0(t0)         # Miss, causes FIFO eviction of oldest block (Tag A)

    # Re-access block with Tag A (Set 0)
    lui t0, 0x00000       # t0 = 0x00000000
    lw  t4, 0(t0)         # Miss again, confirming eviction of Tag A

    # Reload Tag B and Tag C to confirm they are still present
    lui t0, 0x00200       # t0 = 0x00200000
    lw  t5, 0(t0)         # Hit, Tag B present

    lui t0, 0x00400       # t0 = 0x00400000
    lw  t6, 0(t0)         # Hit, Tag C present
