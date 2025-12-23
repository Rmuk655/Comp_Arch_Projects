# Instructions using different functional units
add x1, x2, x3      # Line 1: uses ALU
lw x4, 0(x5)        # Line 2: uses Memory Access Unit (no conflict)
mul x6, x7, x8      # Line 3: uses Mul/Div Unit (no conflict)
sw x9, 4(x10)       # Line 4: uses Memory Access Unit (no conflict with line 2)