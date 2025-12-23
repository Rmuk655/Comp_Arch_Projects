# Scenario: WRITE HIT - WT + No Allocate (LRU)
# Expected: Write to cache + memory. LRU updated.
# Step 0: Setup base address
lui t0, 0x1000          # t0 = 0x10000000

# Step 1: Load block 1 (miss, inserted into set)
lw t1, 0(t0)            # addr 0x10000000 → Tag 0x8000

# Step 2: Load block 2 (miss, inserted into set)
addi t0, t0, 512        # t0 = 0x10000200
lw t2, 0(t0)            # addr 0x10000200 → Tag 0x8001

# Step 3: Load block 3 (miss, evict LRU block — 0x8000 if policy works)
addi t0, t0, 512        # t0 = 0x10000400
lw t3, 0(t0)            # addr 0x10000400 → Tag 0x8002

# Step 4: Reload block 3 → should be a **HIT**
lw t4, 0(t0)            # addr 0x10000400

# Step 5: Write to block 3 → should be a **HIT**
sw t5, 0(t0)            # write to 0x10000400 (Tag 0x8002)
