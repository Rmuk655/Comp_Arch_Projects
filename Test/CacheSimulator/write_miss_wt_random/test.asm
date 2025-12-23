# Scenario: WRITE MISS — Write-Through + No-Write-Allocate (Random)
# Expected: Write directly to memory. Cache unchanged. No eviction.

# Step 0: Setup base address
lui t0, 0x1000          # t0 = 0x10000000

# Step 1: Load block 1 → Miss → insert into set[0]
lw t1, 0(t0)            # addr 0x10000000 → Tag 0x8000

# Step 2: Load block 2 → Miss → insert into set[0]
addi t0, t0, 512        # t0 = 0x10000200
lw t2, 0(t0)            # addr 0x10000200 → Tag 0x8001

# Step 3: Load block 3 → Miss → random eviction of one block
addi t0, t0, 512        # t0 = 0x10000400
lw t3, 0(t0)            # addr 0x10000400 → Tag 0x8002

# Step 4: Write to *new* block → MISS → no allocation
addi t0, t0, 512        # t0 = 0x10000600
sw t5, 0(t0)            # addr 0x10000600 → Tag 0x8003 → Write MISS
