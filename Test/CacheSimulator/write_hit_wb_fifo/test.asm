# Scenario: WRITE HIT - WB + Allocate (FIFO)
# Expected: Dirty block written to memory during eviction
# Write to 0x1000 (Set 0, Tag 0x08)
    lui     t0, 0x1        # t0 = 0x1000
    lw      t1, 0(t0)      # miss - brings block into cache
    addi    t2, zero, 20
    sw      t2, 0(t0)      # write - hit, mark block dirty

# Access 0x9000 (Set 0, Tag 0x48) → fills second block in Set 0
    lui     t3, 0x9        # t3 = 0x9000
    lw      t4, 0(t3)      # miss - insert second block in Set 0

# Access 0xA000 (Set 0, Tag 0x50) → eviction in Set 0
    lui     t5, 0xA        # t5 = 0xA000
    lw      t6, 0(t5)      # miss - evicts oldest (0x1000), should trigger write-back
