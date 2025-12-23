# ALU instructions with immediates still use ALU unit
addi x1, x2, 100    # Line 1: uses ALU
addi x3, x4, -50    # Line 2: structural stall (uses ALU)
andi x5, x6, 0xFF   # Line 3: structural stall (uses ALU)
ori  x7, x8, 0x1F   # Line 4: structural stall (uses ALU)