    addi x1, x0, 1       # Initialize x1 = 1

    # Fill set 0 with 4 blocks (addresses spaced by 0x40 = 64 bytes)
    sw x1, 0x00(x0)      # Write to addr 0x00 → Set 0
    sw x1, 0x40(x0)      # Write to addr 0x40 → Set 0
    sw x1, 0x80(x0)      # Write to addr 0x80 → Set 0
    sw x1, 0xC0(x0)      # Write to addr 0xC0 → Set 0

    # Read some addresses to establish initial cache state
    lw x2, 0x00(x0)      # Read addr 0x00 → should be hit after writes
    lw x3, 0x40(x0)      # Read addr 0x40 → hit

    # Trigger eviction by adding 5th block to same set
    sw x1, 0x100(x0)     # Write addr 0x100 → Set 0 → eviction should happen here

    # Now read all original 4 addresses to see which one got evicted
    lw x2, 0x00(x0)      # Read addr 0x00
    lw x3, 0x40(x0)      # Read addr 0x40
    lw x4, 0x80(x0)      # Read addr 0x80
    lw x5, 0xC0(x0)      # Read addr 0xC0
