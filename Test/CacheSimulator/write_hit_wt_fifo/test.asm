# Cache Test: WRITE HIT - Write-Through + No Allocate (FIFO replacement)
# Expected: Write to cache + memory. FIFO unchanged.

# Step 0: Setup base address
lui t0, 0x1000          # t0 = 0x10000000

# Step 1: Load block 1 → Miss → added to set[0]
lw t1, 0(t0)            # addr 0x10000000 → Tag 0x8000

# Step 2: Load block 2 → Miss → added to set[0]
addi t0, t0, 512        # t0 = 0x10000200
lw t2, 0(t0)            # addr 0x10000200 → Tag 0x8001

# Step 3: Load block 3 → Miss → FIFO eviction of block 1 (0x8000)
addi t0, t0, 512        # t0 = 0x10000400
lw t3, 0(t0)            # addr 0x10000400 → Tag 0x8002

# Step 4: Reload block 3 → should be a **HIT**
lw t4, 0(t0)            # addr 0x10000400 again → Tag 0x8002 (should still be in cache)
sw t5, 0(t0)            # Write to 0x1000400 → should be a HIT if t0 not changed
