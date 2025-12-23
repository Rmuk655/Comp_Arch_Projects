# Scenario: WRITE MISS - WT + No Allocate (FIFO)
# Expected: Write directly to memory.
    # Base address
    lui t0, 0x1000          # t0 = 0x10000000

    # Step 1: Load block A (addr 0x10000000)
    lw t1, 0(t0)            # miss, insert A

    # Step 2: Load block B (addr 0x10000200)
    addi t0, t0, 512
    lw t2, 0(t0)            # miss, insert B

    # Step 3: Access block A again (addr 0x10000000)
    addi t0, t0, -512       # t0 back to 0x10000000
    lw t3, 0(t0)            # hit, updates LRU info

    # Step 4: Load block C (addr 0x10000400) → triggers eviction
    addi t0, t0, 1024       # move to 0x10000400
    lw t4, 0(t0)            # miss, evict block according to FIFO or LRU

    # Step 5: Write to block C (write hit)
    sw t5, 0(t0)

    # Program ends here (no jumps or branches)
