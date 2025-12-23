# Mix of load and store instructions
lw x1, 0(x2)        # Line 1: uses Memory Access Unit (load)
sw x3, 4(x4)        # Line 2: needs Memory Access Unit (store) - structural stall
lh x5, 8(x6)        # Line 3: another load - structural stall
sh x7, 12(x8)       # Line 4: another store - structural stall