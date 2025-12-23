# Scenario: WRITE - Evict Dirty Block (WB)
# Expected: Dirty block written to memory during eviction.
    lui   t0, 0x1
    lw    t1, 0(t0)         # load block a at 0x1000
    addi  t2, zero, 20
    sw    t2, 0(t0)         # write dirty in cache
    lui   t3, 0x1
    lw    t4, 16(t3)        # load block b at 0x1010, evicts dirty block a
    lw    t5, 32(t3)        # load block c at 0x1020, forces eviction if 2-way set
    # Expect dirty block at 0x1000 written back to memory with 20
