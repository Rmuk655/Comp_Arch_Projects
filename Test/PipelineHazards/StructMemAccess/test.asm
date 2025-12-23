# Various load and store instruction types
ld x1, 0(x2)        # Line 1: uses Memory Access Unit (64-bit load)
lbu x3, 1(x4)       # Line 2: structural stall (byte load unsigned)
lhu x5, 2(x6)       # Line 3: structural stall (halfword load unsigned)
sd x7, 8(x8)        # Line 4: structural stall (64-bit store)