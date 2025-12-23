# Initial writes to cold addresses
sw x1, 0(x0)     # Write to address 0x0
sw x1, 16(x0)    # Write to 0x10
sw x1, 32(x0)    # Write to 0x20
sw x1, 48(x0)    # Write to 0x30

# Read back same addresses (test for WA vs NWA, and reuse for WB vs WT)
lw x2, 0(x0)
lw x2, 16(x0)
lw x2, 32(x0)
lw x2, 48(x0)

# Cause eviction: access 5th address in same set
sw x1, 64(x0)    # Write to 0x40, same set if index bits limited

# Read original address again to see if it was evicted
lw x2, 0(x0)
