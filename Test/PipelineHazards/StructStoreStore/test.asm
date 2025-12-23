# Two store instructions competing for memory access unit
sw x1, 0(x2)        # Line 1: uses Memory Access Unit
sw x3, 4(x4)        # Line 2: needs Memory Access Unit (structural stall)
sub x5, x6, x7      # Line 3: can proceed (uses ALU)
sb x8, 8(x9)        # Line 4: another memory access conflict