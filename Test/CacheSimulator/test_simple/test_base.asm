    addi x1, x0, 1        # x1 = 1

    # Insert 4 blocks into set 0:
    # Addresses: 0x00, 0x40, 0x80, 0xC0 all map to set 0
    sw x1, 0x00(x0)       # Write 1 to addr 0x00
    sw x1, 0x40(x0)       # Write 1 to addr 0x40
    sw x1, 0x80(x0)       # Write 1 to addr 0x80
    sw x1, 0xC0(x0)       # Write 1 to addr 0xC0

    # Access 0x00 again (hit on both FIFO/LRU, also refreshes LRU)
    lw x2, 0x00(x0)

    # Access 0x40 again (hit on both FIFO/LRU, refresh LRU for this too)
    lw x2, 0x40(x0)

    # Insert new block at 0x100 (maps to set 0)
    sw x1, 0x100(x0)      # Causes eviction in set 0

    # Now access 0x80
    lw x2, 0x80(x0)       # Hit or miss depends on eviction policy

    # Access 0xC0
    lw x3, 0xC0(x0)       # Hit or miss depends on eviction policy

    # Access 0x00 again
    lw x4, 0x00(x0)       # Hit or miss depends on eviction policy

    # Access 0x40 again
    lw x5, 0x40(x0)       # Hit or miss depends on eviction policy
